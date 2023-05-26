import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "WaylandEglClientHwIntegration"
    Depends { name: "Qt"; submodules: ["core","gui","opengl-private","waylandclient-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["dl","EGL","xkbcommon","wayland-egl"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6WaylandEglClientHwIntegration"
    libNameForLinkerRelease: "Qt6WaylandEglClientHwIntegration"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6WaylandEglClientHwIntegration.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_WAYLAND_EGL_CLIENT_HW_INTEGRATION_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtWaylandEglClientHwIntegration","/home/saksham/Qt/6.5.0/gcc_64/include/QtWaylandEglClientHwIntegration/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtWaylandEglClientHwIntegration/6.5.0/QtWaylandEglClientHwIntegration"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["wayland_egl_client_hw_integration-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
