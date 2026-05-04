cmake_minimum_required(VERSION 3.16)
project(TaskScript LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if (WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    if (EXISTS "C:/Qt/6.11.0/mingw_64")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.11.0/mingw_64")
    endif()
endif()

find_package(Qt6 COMPONENTS Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_LIBRARIES Qt5::Widgets)
else()
    set(QT_LIBRARIES Qt6::Widgets)
endif()

add_executable(TaskScript
    token.h
    lexicalanalyzer.h
    lexicalanalyzer.cpp
    syntaxanalyzer.h
    syntaxanalyzer.cpp
    errormanager.h
    errormanager.cpp
    reportgenerator.h
    reportgenerator.cpp
    mainwindow.h
    mainwindow.cpp
    main.cpp
)

target_link_libraries(TaskScript ${QT_LIBRARIES})