cmake_minimum_required(VERSION 3.16)
project(quickjs C)

# Default to Release (original Makefile: CFLAGS_OPT=-O2)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- version macro (Makefile: DEFINES += -DCONFIG_VERSION="$(VERSION)") ----
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" QUICKJS_VERSION)
string(STRIP "${QUICKJS_VERSION}" QUICKJS_VERSION)
add_compile_definitions(_GNU_SOURCE CONFIG_VERSION="${QUICKJS_VERSION}")

# HAVE_CLOSEFROM probe (Makefile:161-163)
include(CheckCSourceCompiles)
check_c_source_compiles("
#include <unistd.h>
int main(void) { closefrom(0); return 0; }
" HAVE_CLOSEFROM)
if(HAVE_CLOSEFROM)
  add_compile_definitions(HAVE_CLOSEFROM)
endif()

# ---- warnings / codegen flags (Makefile CONFIG_CLANG branch) ----
add_compile_options(
  -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers
  -Wundef -Wuninitialized -Wunused -Wno-unused-parameter
  -Wwrite-strings -Wchar-subscripts
  -funsigned-char -fwrapv
)
# No CMAKE_C_STANDARD: original Makefile sets none (GNU extensions in use).

# ---- LTO/IPO: keep cross-module inlining after the quickjs.c split ----
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

# ---- static library: libquickjs.a ----
# src/*.c GLOB 覆盖核心 16 模块 + cutils/dtoa/libregexp/libunicode/
# quickjs-libc + repl.c（qjsc 生成物，gitignored）
file(GLOB QUICKJS_CORE_SOURCES CONFIGURE_DEPENDS src/*.c)
file(GLOB QUICKJS_BUILTIN_SOURCES CONFIGURE_DEPENDS src/builtins/*.c)
add_library(quickjs STATIC
  ${QUICKJS_CORE_SOURCES}
  ${QUICKJS_BUILTIN_SOURCES}
)
# include/ 存放全部头（quickjs.h 公共 API + 内部头 + builtins/ 子结构）；
# 无系统头同名文件（string.h 隐患已改名 cstring.h / jstring.h），
# 故 -I include 安全。根目录入口（qjs.c/qjsc.c/run-test262.c）通过链接
# 本库获得 PUBLIC include path 解析 quickjs.h/cutils.h 等。
target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---- link libs (Makefile:256-261 LIBS=-lm -lpthread, + -ldl unless WIN32) ----
if(WIN32)
  set(QUICKJS_LIBS quickjs m pthread)
else()
  set(QUICKJS_LIBS quickjs m pthread dl)
endif()

# ---- qjs CLI ----
# repl.o 已随 src/repl.c 进入静态库（qjsc_repl/qjsc_repl_size 符号）
add_executable(qjs qjs.c)
target_link_libraries(qjs PRIVATE ${QUICKJS_LIBS})

# ---- qjsc compiler ----
add_executable(qjsc qjsc.c)
# Makefile: QJSC_CC=$(CC) —— qjsc 产出的构建脚本引用本机构建器，勿写死
target_compile_definitions(qjsc PRIVATE
  CONFIG_CC="${CMAKE_C_COMPILER}" CONFIG_PREFIX="/usr/local")
target_link_libraries(qjsc PRIVATE ${QUICKJS_LIBS})

# ---- run-test262 (optional; cmake --build build --target run-test262) ----
add_executable(run-test262 EXCLUDE_FROM_ALL run-test262.c)
target_link_libraries(run-test262 PRIVATE ${QUICKJS_LIBS})

# ---- tests (Makefile `make test` equivalent) ----
enable_testing()
set(QJS $<TARGET_FILE:qjs>)
function(qjs_test name)
  add_test(NAME ${name}
    COMMAND ${QJS} ${ARGN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()
qjs_test(test_closure       tests/test_closure.js)
qjs_test(test_language      tests/test_language.js)
qjs_test(test_builtin       --std tests/test_builtin.js)
qjs_test(test_loop          tests/test_loop.js)
qjs_test(test_bigint        tests/test_bigint.js)
qjs_test(test_cyclic_import tests/test_cyclic_import.js)
qjs_test(test_worker        tests/test_worker.js)
qjs_test(test_std           tests/test_std.js)
qjs_test(test_rw_handler    tests/test_rw_handler.js)
qjs_test(microbench         --std tests/microbench.js)
set_tests_properties(microbench PROPERTIES TIMEOUT 600)
