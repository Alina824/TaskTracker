cmake_minimum_required(VERSION 3.16)
project(TaskTrackerAndKalendar VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.h
    mainwindow.cpp
    database.h
    database.cpp
    authdialog.h
    authdialog.cpp
    weekview.h
    weekview.cpp
    dayview.h
    dayview.cpp
    monthview.h
    monthview.cpp
    taskdialog.h
    taskdialog.cpp
    notesview.h
    notesview.cpp
    noteeditview.h
    noteeditview.cpp
    allnotesview.h
    allnotesview.cpp
    trackersview.h
    trackersview.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

