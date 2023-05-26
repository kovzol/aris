import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlWorkerScript"
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
    libNameForLinkerDebug: "Qt6QmlWorkerScript"
    libNameForLinkerRelease: "Qt6QmlWorkerScript"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlWorkerScript.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QMLWORKERSCRIPT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlWorkerScript"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["qmlworkerscript"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
