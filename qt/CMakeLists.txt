cmake_minimum_required(VERSION 3.16)

project(aris-qt VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(WITH_CMAKE)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets QuickControls2 LinguistTools)
set(Qt6_FIND_VERSION_MIN 6.0)
find_package(LibXml2 REQUIRED)

add_executable(aris-qt
    ../src/aio.h
    auxconnector.h
    connector.h
    goaldata.h
    goalmodel.h
    goals-qt.h
    settings.h
    ../src/interop-isar.h
    ../src/list.h
    ../src/process.h
    ../src/proof.h
    proofdata.h
    proofmodel.h
    ../src/rules.h
    ../src/sen-data.h
    ../src/sexpr-process.h
    ../src/typedef.h
    ../src/var.h
    ../src/vec.h
    ../src/aio.c
    auxconnector.cpp
    connector.cpp
    goaldata.cpp
    goalmodel.cpp
    goals-qt.c
    settings.cpp
    ../src/interop-isar.c
    ../src/list.c
    main.cpp
    ../src/process-main.c
    ../src/process.c
    ../src/proof.c
    proofdata.cpp
    proofmodel.cpp
    ../src/sen-data.c
    ../src/sexpr-process-bool.c
    ../src/sexpr-process-equiv.c
    ../src/sexpr-process-infer.c
    ../src/sexpr-process-misc.c
    ../src/sexpr-process-quant.c
    ../src/sexpr-process.c
    ../src/var.c
    ../src/vec.c
)

qt_add_qml_module(aris-qt
    URI arisqt
    VERSION 1.0
    QML_FILES main.qml ProofArea.qml DrawerTools.qml GoalLine.qml KeyButton.qml KeyGroup.qml
    RESOURCES
        assets/icon_simple.svg
        assets/icons/arisqt/index.theme
        assets/icons/arisqt/512x512/help.png
        assets/icons/arisqt/512x512/about.png
        assets/icons/arisqt/512x512/font.png
        assets/icons/arisqt/512x512/import.png
        assets/icons/arisqt/512x512/language.png
        assets/icons/arisqt/512x512/boolean.png
        assets/icons/arisqt/512x512/goal.png
        assets/icons/arisqt/512x512/export.png
        assets/icons/arisqt/512x512/saveas.png
        assets/icons/arisqt/512x512/save.png
        assets/icons/arisqt/512x512/folder.png
        assets/icons/arisqt/512x512/new.png
        assets/icons/arisqt/512x512/eval.png
)

qt_add_translations(aris-qt
    TS_FILES
        i18n/aris-qt_ar.ts
        i18n/aris-qt_en.ts)

set_target_properties(aris-qt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(aris-qt
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::QuickControls2
    PUBLIC LibXml2::LibXml2
)
target_link_libraries(aris-qt PRIVATE Qt6::Core)

install(TARGETS aris-qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
