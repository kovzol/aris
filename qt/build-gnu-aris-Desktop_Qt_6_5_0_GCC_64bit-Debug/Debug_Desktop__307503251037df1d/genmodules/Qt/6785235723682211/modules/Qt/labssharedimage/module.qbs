import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsSharedImage"
    Depends { name: "Qt"; submodules: ["core-private","gui-private","quick-private"]}

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
    libNameForLinkerDebug: "Qt6LabsSharedImage"
    libNameForLinkerRelease: "Qt6LabsSharedImage"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6LabsSharedImage.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_LABSSHAREDIMAGE_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsSharedImage"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["labssharedimage"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
