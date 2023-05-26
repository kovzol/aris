import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickDialogs2"
    Depends { name: "Qt"; submodules: ["core","gui","quick"]}

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
    libNameForLinkerDebug: "Qt6QuickDialogs2"
    libNameForLinkerRelease: "Qt6QuickDialogs2"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickDialogs2.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKDIALOGS2_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickDialogs2"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickdialogs2"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
