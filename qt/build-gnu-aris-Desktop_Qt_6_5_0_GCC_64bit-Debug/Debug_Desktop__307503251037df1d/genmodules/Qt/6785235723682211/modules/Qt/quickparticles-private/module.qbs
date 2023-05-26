import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "QuickParticles"
    Depends { name: "Qt"; submodules: ["core-private","gui-private","qml-private","quick-private"]}

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
    libNameForLinkerDebug: "Qt6QuickParticles"
    libNameForLinkerRelease: "Qt6QuickParticles"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6QuickParticles.so"
    pluginTypes: []
    moduleConfig: ["shared"]
    cpp.defines: ["QT_QUICKPARTICLES_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickParticles","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickParticles/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtQuickParticles/6.5.0/QtQuickParticles"]
    cpp.libraryPaths: []
    Group {
        files: [Qt["quickparticles-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
    }
}
