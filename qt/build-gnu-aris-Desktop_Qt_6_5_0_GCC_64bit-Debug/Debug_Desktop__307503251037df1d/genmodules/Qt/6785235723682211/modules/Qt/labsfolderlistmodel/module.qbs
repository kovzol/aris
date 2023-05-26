import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsFolderListModel"
    Depends { name: "Qt"; submodules: ["core-private","qml-private","qmlmodels-private"]}

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
    libNameForLinkerDebug: "Qt6LabsFolderListModel"
    libNameForLinkerRelease: "Qt6LabsFolderListModel"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6LabsFolderListModel.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_LABSFOLDERLISTMODEL_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsFolderListModel"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["labsfolderlistmodel"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
