import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "XcbQpa"
    Depends { name: "Qt"; submodules: ["core-private","gui-private","opengl-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["xkbcommon-x11","xkbcommon","xcb-cursor","xcb-icccm","xcb-image","xcb-keysyms","xcb-randr","xcb-render-util","xcb-shm","xcb-sync","xcb-xfixes","xcb-render","xcb-shape","xcb-xkb","xcb","xkbcommon","X11-xcb","X11","dl"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6XcbQpa"
    libNameForLinkerRelease: "Qt6XcbQpa"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6XcbQpa.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_XCB_QPA_LIB_LIB"]
    cpp.systemIncludePaths: []
    cpp.libraryPaths: []
    Group {
        files: [Qt["xcb_qpa_lib-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
