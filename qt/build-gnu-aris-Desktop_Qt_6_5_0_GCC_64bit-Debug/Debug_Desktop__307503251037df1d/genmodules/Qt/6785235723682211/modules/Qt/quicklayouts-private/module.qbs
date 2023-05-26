import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickLayouts"
    Depends { name: "Qt"; submodules: ["quicklayouts"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: false
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
    libNameForLinkerDebug: undefined
    libNameForLinkerRelease: undefined
    libFilePathDebug: undefined
    libFilePathRelease: undefined
    pluginTypes: []
    moduleConfig: []
    cpp.defines: []
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickLayouts/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickLayouts/6.5.0/QtQuickLayouts"]
    cpp.libraryPaths: []
    
}
