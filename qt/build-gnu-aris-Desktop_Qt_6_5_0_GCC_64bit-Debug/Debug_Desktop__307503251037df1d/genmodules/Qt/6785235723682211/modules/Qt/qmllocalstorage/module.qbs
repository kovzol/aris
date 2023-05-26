import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlLocalStorage"
    Depends { name: "Qt"; submodules: ["core-private","qml-private","sql"]}

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
    libNameForLinkerDebug: "Qt6QmlLocalStorage"
    libNameForLinkerRelease: "Qt6QmlLocalStorage"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlLocalStorage.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QMLLOCALSTORAGE_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlLocalStorage"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["qmllocalstorage"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
