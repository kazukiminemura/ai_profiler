cmake_minimum_required(VERSION 3.21)
project(ai_profiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_add_executable(ai_profiler
    src/main.cpp
    src/profilercontroller.h
    src/profilercontroller.cpp
    src/countersampler.h
    src/countersampler.cpp
    src/cpusampler.h
    src/cpusampler.cpp
    src/aggregator.h
    src/aggregator.cpp
    src/common/datamodels.h
    src/models/timelinemodel.h
    src/models/timelinemodel.cpp
    src/models/hotspotmodel.h
    src/models/hotspotmodel.cpp
    qml/qml.qrc
)

target_include_directories(ai_profiler PRIVATE src)

target_link_libraries(ai_profiler PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)

if (WIN32)
    target_link_libraries(ai_profiler PRIVATE Psapi Pdh)
endif()
