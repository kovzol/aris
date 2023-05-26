import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "WaylandClient"
    Depends { name: "Qt"; submodules: ["core","gui"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["wayland-client","wayland-cursor"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6WaylandClient"
    libNameForLinkerRelease: "Qt6WaylandClient"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6WaylandClient.so"
    pluginTypes: ["wayland-graphics-integration-client","wayland-inputdevice-integration","wayland-decoration-client","wayland-shell-integration"]
    moduleConfig: ["shared"]
    cpp.defines: ["QT_WAYLANDCLIENT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtWaylandClient"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["waylandclient"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
