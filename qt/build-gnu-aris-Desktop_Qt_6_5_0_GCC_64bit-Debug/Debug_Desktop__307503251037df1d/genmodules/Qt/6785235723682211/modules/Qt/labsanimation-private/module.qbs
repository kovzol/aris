import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsAnimation"
    Depends { name: "Qt"; submodules: ["labsanimation"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: false
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
    libNameForLinkerDebug: undefined
    libNameForLinkerRelease: undefined
    libFilePathDebug: undefined
    libFilePathRelease: undefined
    pluginTypes: []
    moduleConfig: []
    cpp.defines: []
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsAnimation/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsAnimation/6.5.0/QtLabsAnimation"]
    cpp.libraryPaths: []
    
}
