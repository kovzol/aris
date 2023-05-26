import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickControls2Impl"
    Depends { name: "Qt"; submodules: ["core","gui","quick"]}

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
    libNameForLinkerDebug: "Qt6QuickControls2Impl"
    libNameForLinkerRelease: "Qt6QuickControls2Impl"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickControls2Impl.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKCONTROLS2IMPL_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickControls2Impl"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickcontrols2impl"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
