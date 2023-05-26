import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "KmsSupport"
    Depends { name: "Qt"; submodules: ["core-private","gui-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: ["drm","xkbcommon"]
    dynamicLibsDebug: []
    dynamicLibsRelease: []
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6KmsSupport"
    libNameForLinkerRelease: "Qt6KmsSupport"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6KmsSupport.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_KMS_SUPPORT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtKmsSupport","/home/saksham/Qt/6.5.0/gcc_64/include/QtKmsSupport/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtKmsSupport/6.5.0/QtKmsSupport"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["kms_support-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
