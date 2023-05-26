import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "PrintSupport"
    Depends { name: "Qt"; submodules: ["core","gui","widgets"]}

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
    libNameForLinkerDebug: "Qt6PrintSupport"
    libNameForLinkerRelease: "Qt6PrintSupport"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6PrintSupport.so"
    pluginTypes: ["printsupport"]
    moduleConfig: ["shared"]
    cpp.defines: ["QT_PRINTSUPPORT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtPrintSupport"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["printsupport"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
