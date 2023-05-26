import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickTestUtils"
    Depends { name: "Qt"; submodules: ["core","testlib","network","qml","qml-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: ["GL","xkbcommon"]
    dynamicLibsDebug: []
    dynamicLibsRelease: []
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6QuickTestUtils"
    libNameForLinkerRelease: "Qt6QuickTestUtils"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickTestUtils.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_QUICKTESTUTILSPRIVATE_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickTestUtils","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickTestUtils/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickTestUtils/6.5.0/QtQuickTestUtils"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["quicktestutilsprivate-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
