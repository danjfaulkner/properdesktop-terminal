cmake_minimum_required(VERSION 3.16)

project(properdesktop-terminal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(ProperDesktopToolkit REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(lib)

add_executable(${PROJECT_NAME}
    src/findbar.cpp
    src/main.cpp
    src/processhelper.cpp
    src/settingsdialog.cpp
    src/terminalpage.cpp
    src/terminalsettings.cpp
    src/terminalwindow.cpp
    src/terminalwindowdialogs.cpp
    src/windows.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE terminal-display)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY shell-integration DESTINATION ${CMAKE_INSTALL_DATADIR}/properdesktop-terminal)

properdesktop_add_translations(properdesktop-terminal SOURCES src DESKTOP_FILES properdesktop-terminal.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/properdesktop-terminal.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
