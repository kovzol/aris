import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "InputSupport"
    Depends { name: "Qt"; submodules: ["core-private","devicediscovery_support-private","gui-private"]}

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
    libNameForLinkerDebug: "Qt6InputSupport"
    libNameForLinkerRelease: "Qt6InputSupport"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6InputSupport.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_INPUT_SUPPORT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtInputSupport","/home/saksham/Qt/6.5.0/gcc_64/include/QtInputSupport/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtInputSupport/6.5.0/QtInputSupport"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["input_support-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
