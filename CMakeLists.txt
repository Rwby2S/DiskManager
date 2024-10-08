cmake_minimum_required(VERSION 3.5)
project(DiskManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)

set(PROJECT_SOURCES
        main.cpp
        fileanalyzer.cpp
        fileanalyzer.h
        fileanalyzer.ui
        resources.qrc
        charthandler.cpp
        charthandler.h
        filedetailswidget.cpp
        filedetailswidget.h
        filedetailswidget.ui
        FileScanner.cpp
        FileScanner.h
        FileSystemHandler.cpp
        FileSystemHandler.h
        layeredfilescanner.cpp
        layeredfilescanner.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DiskManager
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
        fileanalyzer.h fileanalyzer.cpp fileanalyzer.ui
        FileScanner.h FileScanner.cpp
        FileSystemHandler.h FileSystemHandler.cpp
        charthandler.h charthandler.cpp
        filedetailswidget.h filedetailswidget.cpp
        layeredfilescanner.h layeredfilescanner.cpp
        contextmenuhandler.h contextmenuhandler.cpp
    )
else()
    if(ANDROID)
        add_library(DiskManager SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(DiskManager
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(DiskManager PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DiskManager)
endif()

set_target_properties(DiskManager PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DiskManager
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DiskManager)
endif()
