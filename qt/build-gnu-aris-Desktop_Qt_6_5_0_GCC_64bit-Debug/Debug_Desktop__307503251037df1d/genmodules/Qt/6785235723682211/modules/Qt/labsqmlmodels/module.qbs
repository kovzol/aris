import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsQmlModels"
    Depends { name: "Qt"; submodules: ["qmlmodels-private","qml-private"]}

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
    libNameForLinkerDebug: "Qt6LabsQmlModels"
    libNameForLinkerRelease: "Qt6LabsQmlModels"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6LabsQmlModels.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_LABSQMLMODELS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsQmlModels"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["labsqmlmodels"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
