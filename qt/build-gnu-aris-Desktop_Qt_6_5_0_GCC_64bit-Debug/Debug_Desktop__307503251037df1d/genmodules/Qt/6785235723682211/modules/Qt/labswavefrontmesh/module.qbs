import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "LabsWavefrontMesh"
    Depends { name: "Qt"; submodules: ["core-private","gui-private","quick-private"]}

    architectures: ["x86_64"]
    targetPlatform: "linux"
    hasLibrary: true
    staticLibsDebug: []
    staticLibsRelease: []
    dynamicLibsDebug: []
    dynamicLibsRelease: ["xkbcommon"]
    linkerFlagsDebug: []
    linkerFlagsRelease: []
    frameworksDebug: []
    frameworksRelease: []
    frameworkPathsDebug: []
    frameworkPathsRelease: []
    libNameForLinkerDebug: "Qt6LabsWavefrontMesh"
    libNameForLinkerRelease: "Qt6LabsWavefrontMesh"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6LabsWavefrontMesh.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_LABSWAVEFRONTMESH_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtLabsWavefrontMesh"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["labswavefrontmesh"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
