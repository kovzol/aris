import qbs
CppApplication {
    Depends { name: "Qt.quick" }
    install: true
    // Additional import path used to resolve QML modules in Qt Creator's code model
    property pathList qmlImportPaths: []

    files: [
        "main.cpp",
        "resources.qrc",
    ]

    Group {
        Qt.core.resourcePrefix: "GNU-Aris/"
        fileTags: ["qt.qml.qml", "qt.core.resource_data"]
        files: [
            "Main.qml",
        ]
    }
}
