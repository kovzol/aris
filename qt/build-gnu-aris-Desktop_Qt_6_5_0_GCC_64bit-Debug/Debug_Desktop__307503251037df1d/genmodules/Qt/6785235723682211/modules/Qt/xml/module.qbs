import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "Xml"
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
    libNameForLinkerDebug: "Qt6Xml"
    libNameForLinkerRelease: "Qt6Xml"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6Xml.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_XML_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtXml"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["xml"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
