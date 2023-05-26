import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsSettings"
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
    libNameForLinkerDebug: "Qt6LabsSettings"
    libNameForLinkerRelease: "Qt6LabsSettings"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6LabsSettings.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_LABSSETTINGS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsSettings"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["labssettings"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
