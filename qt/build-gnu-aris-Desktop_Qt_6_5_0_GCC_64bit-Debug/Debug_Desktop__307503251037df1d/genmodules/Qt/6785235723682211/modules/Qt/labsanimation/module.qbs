import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsAnimation"
    Depends { name: "Qt"; submodules: ["qml-private","quick-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["xkbcommon"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6LabsAnimation"
    libNameForLinkerRelease: "Qt6LabsAnimation"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6LabsAnimation.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_LABSANIMATION_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsAnimation"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["labsanimation"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
