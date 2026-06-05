cmake_minimum_required(VERSION 3.16)
project(TypingSpeedTest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt ──────────────────────────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Core Widgets)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    message(STATUS "Using Qt5")
else()
    message(STATUS "Using Qt6")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Sources ─────────────────────────────────────────────────
set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

add_executable(TypingSpeedTest WIN32
    ${SOURCES}
    ${HEADERS}
)

# ── Link Qt ─────────────────────────────────────────────────
if(Qt6_FOUND)
    target_link_libraries(TypingSpeedTest PRIVATE Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(TypingSpeedTest PRIVATE Qt5::Core Qt5::Widgets)
endif()

# ── Copy Assembly exe next to Qt app after build ─────────────
add_custom_command(TARGET TypingSpeedTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/asm/typing_analyzer.exe"
        "$<TARGET_FILE_DIR:TypingSpeedTest>/typing_analyzer.exe"
    COMMENT "Copying typing_analyzer.exe to output directory"
    VERBATIM
)
