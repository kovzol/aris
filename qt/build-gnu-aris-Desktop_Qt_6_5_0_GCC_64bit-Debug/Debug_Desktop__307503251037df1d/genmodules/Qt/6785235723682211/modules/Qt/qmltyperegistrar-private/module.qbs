import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QmlTypeRegistrar"
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
    libNameForLinkerDebug: "Qt6QmlTypeRegistrar"
    libNameForLinkerRelease: "Qt6QmlTypeRegistrar"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QmlTypeRegistrar.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_QMLTYPEREGISTRAR_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlTypeRegistrar","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlTypeRegistrar/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQmlTypeRegistrar/6.5.0/QtQmlTypeRegistrar"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["qmltyperegistrar-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
