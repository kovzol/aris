cmake_minimum_required(VERSION 3.16)

project(aris VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(aris
    main.cpp
)

add_subdirectory(GNU/Aris/Widgets/)

qt_add_qml_module(aris
    URI GNU.Aris
    RESOURCE_PREFIX /org.gnu.aris/qml/

    QML_FILES
        Main.qml

    IMPORTS
        GNU.Aris.Widgets
)

set_target_properties(aris PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.gnu.aris
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(aris
    PRIVATE Qt6::Quick
)

install(TARGETS aris
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
