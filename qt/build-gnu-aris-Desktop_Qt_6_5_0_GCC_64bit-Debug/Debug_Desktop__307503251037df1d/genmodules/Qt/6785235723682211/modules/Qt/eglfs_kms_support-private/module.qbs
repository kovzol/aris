import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "EglFsKmsSupport"
    Depends { name: "Qt"; submodules: ["core","core-private","eglfsdeviceintegration-private","gui","gui-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["drm"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6EglFsKmsSupport"
    libNameForLinkerRelease: "Qt6EglFsKmsSupport"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6EglFsKmsSupport.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_EGLFS_KMS_SUPPORT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtEglFsKmsSupport","/home/saksham/Qt/6.5.0/gcc_64/include/QtEglFsKmsSupport/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtEglFsKmsSupport/6.5.0/QtEglFsKmsSupport"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["eglfs_kms_support-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
