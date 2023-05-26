import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickEffects"
    Depends { name: "Qt"; submodules: ["core-private","gui-private","qml-private","quick-private"]}

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
    libNameForLinkerDebug: "Qt6QuickEffects"
    libNameForLinkerRelease: "Qt6QuickEffects"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickEffects.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKEFFECTS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickEffects","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickEffects/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickEffects/6.5.0/QtQuickEffects"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickeffects-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
