import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "Designer"
    Depends { name: "Qt"; submodules: ["core","gui","uiplugin","widgets","xml","openglwidgets"]}

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
    libNameForLinkerDebug: "Qt6Designer"
    libNameForLinkerRelease: "Qt6Designer"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6Designer.so"
    pluginTypes: ["designer"]
    moduleConfig: ["shared"]
    cpp.defines: ["QT_DESIGNER_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtDesigner"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["designer"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
