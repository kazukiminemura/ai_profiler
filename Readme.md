# AI Profiler (Qt6 / Windows MVP)

本リポジトリは、AIアプリ向けのリアルタイム・プロファイリングツール（Intel VTune Hotspots/Timelineのライト版）MVP実装です。Windowsを主ターゲットに、UIはQt Quickで軽量・モダンな見た目を目指しています。

## 1) アーキテクチャ

```
UI(QML)
  ├─ OverviewPage / HotspotsPage
  └─ TimelineModel / HotspotModel (QAbstractListModel)

Controller
  └─ ProfilerController (Start/Stop, 状態管理, エラー処理)

Collector
  ├─ CounterSampler (1Hz): CPU%, RSS, Threads
  └─ CpuSampler (100Hz): スレッド別CPU時間サンプリング

Aggregator
  └─ Aggregator (2s flush): stack_idで集計してTop Nを生成

Transport
  └─ Qt signal/slot (QueuedConnection)
```

## 2) スレッドモデル

- **UIスレッド**: QML描画、モデル更新（TimelineModel/HotspotModel）
- **CounterSamplerスレッド**: 1Hzでプロセスカウンタ取得
- **CpuSamplerスレッド**: 100HzでスレッドCPU時間を取得
- **Aggregatorスレッド**: サンプル集計と2秒ごとのflush

データ競合回避はQtのQueuedConnectionを使用し、UIはメインスレッドでのみ更新します。

## 3) データモデル

- `CounterFrame`:
  - `epochMs`: タイムスタンプ
  - `cpuPercent`: プロセスCPU使用率（%）
  - `rssMB`: RSS (MB)
  - `threadCount`: スレッド数

- `CpuSample`:
  - `epochMs`
  - `threadId`
  - `cpuDelta100ns`: 100ns単位CPU時間差分
  - `stackId`: MVPはthreadIdを仮stackIdとして扱う（将来PDB対応）

- `HotspotFrame`:
  - `epochMs`
  - `entries[]`: `HotspotEntry { stackId, threadId, cpuMs, cpuPercent }`

## 4) ディレクトリ構成

```
ai_profiler/
├─ CMakeLists.txt
├─ Readme.md
├─ src/
│  ├─ main.cpp
│  ├─ profilercontroller.h/.cpp
│  ├─ countersampler.h/.cpp
│  ├─ cpusampler.h/.cpp
│  ├─ aggregator.h/.cpp
│  ├─ common/datamodels.h
│  └─ models/
│     ├─ timelinemodel.h/.cpp
│     └─ hotspotmodel.h/.cpp
└─ qml/
   ├─ qml.qrc
   ├─ Main.qml
   ├─ OverviewPage.qml
   ├─ HotspotsPage.qml
   └─ components/
      ├─ Card.qml
      └─ TimelineGraph.qml
```

## 5) 主要クラス設計

- **ProfilerController**
  - `start(pid)` / `stop()`
  - サンプラ/アグリゲータスレッド制御
  - エラー通知とステータス管理

- **CounterSampler (1Hz)**
  - `GetProcessTimes`, `GetProcessMemoryInfo`, `Toolhelp32Snapshot`で取得

- **CpuSampler (100Hz)**
  - `GetThreadTimes`でスレッドCPU時間差分
  - MVPは`stackId=threadId`（簡易）

- **Aggregator (2s flush)**
  - `stackId`ごとに集計しTop N生成

- **TimelineModel / HotspotModel**
  - UIから直接参照可能なQAbstractListModel

## 6) 最小実装コードの動作概要

- PID入力 → Start → カウンタ(1Hz)とHotspots(2s)がリアルタイム更新
- Stopで安全にワーカースレッド終了
- UIはダークテーマのカードUI＋簡易タイムライン

## 7) ビルド手順 (Windows / Qt6)

```
cmake -S . -B build
cmake --build build --config Release
```

Qt6が見つからない場合は、`CMAKE_PREFIX_PATH`にQt6のパスを指定してください。

## 8) 次の拡張ステップ

1. 保存機能: CounterFrame/HotspotFrameをセッション形式で永続化
2. Flamegraph: stack_id解決後にflamegraph生成
3. ETW/PMU統合: 正確なサンプリング（高精度）
4. GPU/NPU: DX12/DirectML/ONNX Runtimeの実行計測
5. 推論Span: モデル推論区間の可視化（Trace/Span）

---

### 注意点
- MVPではstack解決は簡略化しており、`stackId=threadId`相当です。PDB対応を追加すると、`stackId`を`module+ip`または`hash(module+ip)`に置き換え可能です。
- UIスレッドをブロックしないため、Collector/CPU Sampling/集計は完全にワーカースレッドで動作します。
