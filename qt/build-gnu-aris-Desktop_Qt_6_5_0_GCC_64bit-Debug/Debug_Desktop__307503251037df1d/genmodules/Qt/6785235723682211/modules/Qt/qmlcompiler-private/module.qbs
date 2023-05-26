import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlCompiler"
    Depends { name: "Qt"; submodules: ["core-private","qml-private"]}

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
    libNameForLinkerDebug: "Qt6QmlCompiler"
    libNameForLinkerRelease: "Qt6QmlCompiler"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlCompiler.so"
    pluginTypes: ["qmllint"]
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QMLCOMPILER_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlCompiler","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlCompiler/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlCompiler/6.5.0/QtQmlCompiler"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["qmlcompiler-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
