QT += quick widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        aio.c \
        interop-isar.c \
        list.c \
        main.cpp \
        process-main.c \
        process.c \
        proof.c \
        sen-data.c \
        sexpr-process-bool.c \
        sexpr-process-equiv.c \
        sexpr-process-infer.c \
        sexpr-process-misc.c \
        sexpr-process-quant.c \
        sexpr-process.c \
        test.c \
        var.c \
        vec.c \
        wrapper.cpp

RESOURCES += qml.qrc \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# TODO : Include path for libxml


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    aio.h \
    interop-isar.h \
    list.h \
    process.h \
    proof.h \
    rules.h \
    sen-data.h \
    sexpr-process.h \
    test.h \
    typedef.h \
    var.h \
    vec.h \
    wrapper.h
