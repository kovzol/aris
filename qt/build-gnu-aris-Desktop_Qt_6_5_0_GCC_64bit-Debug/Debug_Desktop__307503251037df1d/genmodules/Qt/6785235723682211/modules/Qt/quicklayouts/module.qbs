import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickLayouts"
    Depends { name: "Qt"; submodules: ["core","gui-private","quick-private","qml"]}

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
    libNameForLinkerDebug: "Qt6QuickLayouts"
    libNameForLinkerRelease: "Qt6QuickLayouts"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickLayouts.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKLAYOUTS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickLayouts"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quicklayouts"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
