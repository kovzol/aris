import '../QtModule.qbs' as QtModule

QtModule {
    qtModuleName: "DeviceDiscoverySupport"
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
    libNameForLinkerDebug: "Qt6DeviceDiscoverySupport"
    libNameForLinkerRelease: "Qt6DeviceDiscoverySupport"
    libFilePathDebug: undefined
    libFilePathRelease: "/home/saksham/Qt/6.5.0/gcc_64/lib/libQt6DeviceDiscoverySupport.a"
    pluginTypes: []
    moduleConfig: ["static"]
    cpp.defines: ["QT_DEVICEDISCOVERY_SUPPORT_LIB"]
    cpp.systemIncludePaths: ["/home/saksham/Qt/6.5.0/gcc_64/include","/home/saksham/Qt/6.5.0/gcc_64/include/QtDeviceDiscoverySupport","/home/saksham/Qt/6.5.0/gcc_64/include/QtDeviceDiscoverySupport/6.5.0","/home/saksham/Qt/6.5.0/gcc_64/include/QtDeviceDiscoverySupport/6.5.0/QtDeviceDiscoverySupport"]
    cpp.libraryPaths: []
    isStaticLibrary: true
    Group {
        files: [Qt["devicediscovery_support-private"].libFilePath]
        filesAreTargets: true
        fileTags: ["staticlibrary"]
    }
}
