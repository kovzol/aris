import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickControls2"
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
    libNameForLinkerDebug: "Qt6QuickControls2"
    libNameForLinkerRelease: "Qt6QuickControls2"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickControls2.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKCONTROLS2_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickControls2"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickcontrols2"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
