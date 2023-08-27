cmake_minimum_required(VERSION 3.10)
project(aris C)

set(PACKAGE_NAME "GNU Aris")
set(PACKAGE "aris")
set(ARIS_VERSION "3.0")
set(PACKAGE_BUGREPORT "aris-bugs@gnu.org")

set(CMAKE_C_STANDARD 11)

add_executable(aris
        src/aio.c
        src/aio.h
        src/app.c
        src/app.h
        src/aris-proof.c
        src/aris-proof.h
        src/aris.c
        src/callbacks.c
        src/callbacks.h
        src/conf-file.c
        src/conf-file.h
        src/config.h
        src/goal.c
        src/goal.h
        src/interop-isar.c
        src/interop-isar.h
        src/list.c
        src/list.h
        src/menu.c
        src/menu.h
        src/pound.h
        src/process-main.c
        src/process.c
        src/process.h
        src/proof.c
        src/proof.h
        src/rules-table.c
        src/rules-table.h
        src/rules.h
        src/sen-data.c
        src/sen-data.h
        src/sen-parent.c
        src/sen-parent.h
        src/sentence.c
        src/sentence.h
	src/sentence-screen-keyboard.h
	src/sentence-screen-keyboard.c
        src/sexpr-process-bool.c
        src/sexpr-process-equiv.c
        src/sexpr-process-infer.c
        src/sexpr-process-misc.c
        src/sexpr-process-quant.c
        src/sexpr-process.c
        src/sexpr-process.h
        src/typedef.h
        src/undo.c
        src/undo.h
        src/var.c
        src/var.h
        src/vec.c
        src/vec.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DARIS_GUI")
configure_file(cmake/config.h.in config.h)
find_package(LibXml2 REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
include_directories(src ${GTK3_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(aris m ${LIBXML2_LIBRARY} ${GTK3_LIBRARIES} gio-2.0)

install(TARGETS aris DESTINATION bin)
