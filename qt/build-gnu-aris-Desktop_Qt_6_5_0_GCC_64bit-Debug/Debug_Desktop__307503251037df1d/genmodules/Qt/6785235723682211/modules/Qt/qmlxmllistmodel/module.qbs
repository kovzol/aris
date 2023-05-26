import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlXmlListModel"
    Depends { name: "Qt"; submodules: ["core","qml"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: []
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6QmlXmlListModel"
    libNameForLinkerRelease: "Qt6QmlXmlListModel"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlXmlListModel.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QMLXMLLISTMODEL_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlXmlListModel"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["qmlxmllistmodel"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
