import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "Concurrent"
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
    libNameForLinkerDebug: "Qt6Concurrent"
    libNameForLinkerRelease: "Qt6Concurrent"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6Concurrent.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_CONCURRENT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtConcurrent"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["concurrent"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
