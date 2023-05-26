import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlCore"
    Depends { name: "Qt"; submodules: ["core","qml"]}

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
    libNameForLinkerDebug: "Qt6QmlCore"
    libNameForLinkerRelease: "Qt6QmlCore"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlCore.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QMLCORE_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlCore"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["qmlcore"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
