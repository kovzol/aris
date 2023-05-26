import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlDom"
    Depends { name: "Qt"; submodules: ["qml-private","qmlcompiler-private"]}

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
    libNameForLinkerDebug: "Qt6QmlDom"
    libNameForLinkerRelease: "Qt6QmlDom"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlDom.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_QMLDOM_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlDom","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlDom/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlDom/6.5.0/QtQmlDom"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["qmldom-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
