import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "qtlibpng"
    Depends { name: "Qt"; submodules: []}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: ["/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6BundledLibpng.a"]
    staticLibsRelease: ["/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6BundledLibpng.a"]
    dynamicLibsDebug: []
    dynamicLibsRelease: []
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "qtlibpng"
    libNameForLinkerRelease: "qtlibpng"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6BundledLibpng.a"
    pluginTypes: []
    moduleConfig: []
    cpp.defines: ["QT_LIBPNG_LIB"]
    cpp.systemIncludePaths: ["/home/qt/work/qt/qtbase/src/3rdparty/libpng"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["libpng"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
