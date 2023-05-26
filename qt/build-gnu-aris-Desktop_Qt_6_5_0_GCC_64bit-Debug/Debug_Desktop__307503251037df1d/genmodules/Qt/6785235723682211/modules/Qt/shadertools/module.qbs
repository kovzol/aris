import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "ShaderTools"
    Depends { name: "Qt"; submodules: ["core","gui"]}

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
    libNameForLinkerDebug: "Qt6ShaderTools"
    libNameForLinkerRelease: "Qt6ShaderTools"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6ShaderTools.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_SHADERTOOLS_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtShaderTools"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["shadertools"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
