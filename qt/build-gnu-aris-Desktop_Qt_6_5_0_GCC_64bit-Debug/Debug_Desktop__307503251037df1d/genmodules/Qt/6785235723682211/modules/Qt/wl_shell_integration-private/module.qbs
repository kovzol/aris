import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "WlShellIntegration"
    Depends { name: "Qt"; submodules: []}

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
    libNameForLinkerDebug: "Qt6WlShellIntegration"
    libNameForLinkerRelease: "Qt6WlShellIntegration"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6WlShellIntegration.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_WL_SHELL_INTEGRATION_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtWlShellIntegration","/home/saksham/Qt/6.5.0/gcc_64/include/QtWlShellIntegration/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtWlShellIntegration/6.5.0/QtWlShellIntegration"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["wl_shell_integration-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
