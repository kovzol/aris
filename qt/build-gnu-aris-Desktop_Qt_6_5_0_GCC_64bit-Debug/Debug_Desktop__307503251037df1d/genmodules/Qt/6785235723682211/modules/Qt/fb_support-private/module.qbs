import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "FbSupport"
    Depends { name: "Qt"; submodules: ["core-private","gui-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: ["xkbcommon"]
    dynamicLibsDebug: []
    dynamicLibsRelease: []
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6FbSupport"
    libNameForLinkerRelease: "Qt6FbSupport"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6FbSupport.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_FB_SUPPORT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtFbSupport","/home/saksham/Qt/6.5.0/gcc_64/include/QtFbSupport/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtFbSupport/6.5.0/QtFbSupport"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["fb_support-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
