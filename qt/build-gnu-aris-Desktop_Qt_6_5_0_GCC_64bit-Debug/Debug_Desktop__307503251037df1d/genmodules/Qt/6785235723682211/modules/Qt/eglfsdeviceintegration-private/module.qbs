import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "EglFSDeviceIntegration"
    Depends { name: "Qt"; submodules: ["core-private","gui-private","opengl-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["EGL","xkbcommon"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6EglFSDeviceIntegration"
    libNameForLinkerRelease: "Qt6EglFSDeviceIntegration"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6EglFSDeviceIntegration.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_EGLFSDEVICEINTEGRATION_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtEglFSDeviceIntegration","/home/saksham/Qt/6.5.0/gcc_64/include/QtEglFSDeviceIntegration/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtEglFSDeviceIntegration/6.5.0/QtEglFSDeviceIntegration"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["eglfsdeviceintegration-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
