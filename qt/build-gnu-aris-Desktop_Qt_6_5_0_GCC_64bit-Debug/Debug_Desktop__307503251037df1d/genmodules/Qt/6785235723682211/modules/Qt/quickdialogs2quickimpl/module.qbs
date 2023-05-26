import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickDialogs2QuickImpl"
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
    libNameForLinkerDebug: "Qt6QuickDialogs2QuickImpl"
    libNameForLinkerRelease: "Qt6QuickDialogs2QuickImpl"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickDialogs2QuickImpl.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKDIALOGS2QUICKIMPL_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickDialogs2QuickImpl"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickdialogs2quickimpl"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
