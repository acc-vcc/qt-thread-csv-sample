cmake_minimum_required(VERSION 3.10)
project(qt_thread_csv_sample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(SRC_FILES
    main.cpp
    MainWindow.cpp
    Worker.cpp
    FileEnumerator.cpp
    CsvParser.cpp
    Logger.cpp
)

set(HDR_FILES
    MainWindow.h
    Worker.h
    FileEnumerator.h
    CsvParser.h
    Logger.h
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${HDR_FILES}
)

# Release ビルド時だけ Debug ログを無効化
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Release>:NO_DEBUG_LOG>
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
)