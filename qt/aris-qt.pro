QT += quick widgets quickcontrols2

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../src/aio.c \
        auxconnector.cpp \
        connector.cpp \
        goaldata.cpp \
        goalmodel.cpp \
        goals-qt.c \
        ../src/interop-isar.c \
        ../src/list.c \
        main.cpp \
        ../src/process-main.c \
        ../src/process.c \
        ../src/proof.c \
        proofdata.cpp \
        proofmodel.cpp \
        ../src/sen-data.c \
        ../src/sexpr-process-bool.c \
        ../src/sexpr-process-equiv.c \
        ../src/sexpr-process-infer.c \
        ../src/sexpr-process-misc.c \
        ../src/sexpr-process-quant.c \
        ../src/sexpr-process.c \
        ../src/var.c \
        ../src/vec.c

RESOURCES += qml.qrc \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Path to mingw
win32: MINGW64_ROOT = C:/msys64/mingw64

# Include path for libxml
!wasm{
    unix: INCLUDEPATH += /usr/include/libxml2
    win32: INCLUDEPATH += $${MINGW64_ROOT}/include/libxml2
}

# Lib for libxml
!wasm{
    unix: LIBS += -lxml2
    win32: LIBS += $${MINGW64_ROOT}/lib/libxml2.a
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../src/aio.h \
    auxconnector.h \
    connector.h \
    goaldata.h \
    goalmodel.h \
    goals-qt.h \
    ../src/interop-isar.h \
    ../src/list.h \
    ../src/process.h \
    ../src/proof.h \
    proofdata.h \
    proofmodel.h \
    ../src/rules.h \
    ../src/sen-data.h \
    ../src/sexpr-process.h \
    ../src/typedef.h \
    ../src/var.h \
    ../src/vec.h

# Uncomment the below lines when compiling to webassembly
wasm{
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libxml2/release/ -lxml2
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libxml2/debug/ -lxml2
    else:unix: LIBS += -L$$PWD/../libxml2/ -lxml2

    INCLUDEPATH += $$PWD/../libxml2
    DEPENDPATH += $$PWD/../libxml2

    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../libxml2/release/libxml2.a
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../libxml2/debug/libxml2.a
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../libxml2/release/xml2.lib
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../libxml2/debug/xml2.lib
    else:unix: PRE_TARGETDEPS += $$PWD/../libxml2/libxml2.a
}
