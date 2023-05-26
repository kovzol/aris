import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "Core5Compat"
    Depends { name: "Qt"; submodules: ["core"]}

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
    libNameForLinkerDebug: "Qt6Core5Compat"
    libNameForLinkerRelease: "Qt6Core5Compat"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6Core5Compat.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_CORE5COMPAT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtCore5Compat"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["core5compat"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
