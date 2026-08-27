qt_add_qml_module(aris-widgets
    URI GNU.Aris.Widgets
    RESOURCE_PREFIX /org.gnu.aris/qml/widgets/
    STATIC

    QML_FILES
        ProofLine/ProofLine.qml
    SOURCES
        ProofLine/HighlighterBridge.cpp
        ProofLine/HighlighterBridge.h
        ProofLine/Highlighter.cpp
        ProofLine/Highlighter.h
)

target_include_directories(aris-widgets
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ProofLine"
)

target_link_libraries(aris-widgets
    PRIVATE Qt6::Quick
)
