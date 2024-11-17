cmake_minimum_required(VERSION 3.5)

project(ScreenCutTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        screen_widget.cpp
        screen_widget.h
        screen_widget.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    set(PROJECT_SOURCES
        ${PROJECT_SOURCES}
        main_window.h main_window.cpp
        app_icon.qrc
        QHotkey/QHotkey
        QHotkey/qhotkey.cpp
        QHotkey/qhotkey.h
        QHotkey/qhotkey_p.h
    )

    # 针对 Windows 平台
    if(WIN32)
        list(APPEND PROJECT_SOURCES QHotkey/qhotkey_win.cpp)
    endif()

    # 针对 macOS 平台
    if(APPLE)
        list(APPEND PROJECT_SOURCES QHotkey/qhotkey_mac.cpp)
    endif()

    # 针对 X11（Linux）平台
    if(UNIX AND NOT APPLE)
        list(APPEND PROJECT_SOURCES QHotkey/qhotkey_x11.cpp)
    endif()

    qt_add_executable(ScreenCutTool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ScreenCutTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ScreenCutTool SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ScreenCutTool
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ScreenCutTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ScreenCutTool)
endif()
set_target_properties(ScreenCutTool PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ScreenCutTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ScreenCutTool)
endif()
