import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "SvgWidgets"
    Depends { name: "Qt"; submodules: ["core","gui","svg","widgets"]}

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
    libNameForLinkerDebug: "Qt6SvgWidgets"
    libNameForLinkerRelease: "Qt6SvgWidgets"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6SvgWidgets.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_SVGWIDGETS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtSvgWidgets"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["svgwidgets"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
