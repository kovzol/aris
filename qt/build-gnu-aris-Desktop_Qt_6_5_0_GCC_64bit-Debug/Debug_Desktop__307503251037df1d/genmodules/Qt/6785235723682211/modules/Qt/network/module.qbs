import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "Network"
    Depends { name: "Qt"; submodules: ["core"]}

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
    libNameForLinkerDebug: "Qt6Network"
    libNameForLinkerRelease: "Qt6Network"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6Network.so"
    pluginTypes: ["networkaccess","networkinformation","tls"]
    moduleConfig: ["shared"]
    cpp.defines: ["QT_NETWORK_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtNetwork"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["network"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
