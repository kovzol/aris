import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickDialogs2Utils"
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
    libNameForLinkerDebug: "Qt6QuickDialogs2Utils"
    libNameForLinkerRelease: "Qt6QuickDialogs2Utils"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickDialogs2Utils.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKDIALOGS2UTILS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickDialogs2Utils"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickdialogs2utils"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
