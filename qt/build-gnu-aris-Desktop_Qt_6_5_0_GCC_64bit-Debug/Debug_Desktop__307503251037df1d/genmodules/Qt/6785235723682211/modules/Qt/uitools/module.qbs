import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "UiTools"
    Depends { name: "Qt"; submodules: ["core","gui","widgets","openglwidgets"]}

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
    libNameForLinkerDebug: "Qt6UiTools"
    libNameForLinkerRelease: "Qt6UiTools"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6UiTools.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_UITOOLS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtUiTools"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["uitools"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
