import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "PacketProtocol"
    Depends { name: "Qt"; submodules: ["core-private"]}

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
    libNameForLinkerDebug: "Qt6PacketProtocol"
    libNameForLinkerRelease: "Qt6PacketProtocol"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6PacketProtocol.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_PACKETPROTOCOL_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtPacketProtocol","/home/saksham/Qt/6.5.0/gcc_64/include/QtPacketProtocol/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtPacketProtocol/6.5.0/QtPacketProtocol"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["packetprotocol-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
