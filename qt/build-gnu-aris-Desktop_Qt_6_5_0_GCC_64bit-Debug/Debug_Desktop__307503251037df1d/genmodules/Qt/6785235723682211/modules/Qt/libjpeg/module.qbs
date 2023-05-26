import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "qtlibjpeg"
    Depends { name: "Qt"; submodules: []}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: ["/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6BundledLibjpeg.a"]
    staticLibsRelease: ["/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6BundledLibjpeg.a"]
    dynamicLibsDebug: []
    dynamicLibsRelease: []
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "qtlibjpeg"
    libNameForLinkerRelease: "qtlibjpeg"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6BundledLibjpeg.a"
    pluginTypes: []
    moduleConfig: []
    cpp.defines: ["QT_LIBJPEG_LIB"]
    cpp.systemIncludePaths: ["/home/qt/work/qt/qtbase/src/3rdparty/libjpeg/src"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["libjpeg"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
