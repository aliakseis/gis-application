add_compile_definitions(CPL_DISABLE_DLL)

set(HEADER_FILES
    cpl_config.h
    cpl_conv.h
    cpl_csv.h
    cpl_error.h
    cpl_minixml.h
    cpl_multiproc.h
    cpl_port.h
    cpl_string.h
    cpl_vsi.h
    cpl_vsi_virtual.h)

set(SOURCE_FILES
    cpl_conv.cpp
    cpl_csv.cpp
    cpl_dir.cpp
    cpl_error.cpp
    cpl_findfile.cpp
    cpl_getexecpath.cpp
    cplgetsymbol.cpp
    cpl_minixml.cpp
    cpl_multiproc.cpp
    cpl_path.cpp
    cpl_string.cpp
    cplstring.cpp
    cpl_vsil.cpp
    cpl_vsil_unix_stdio_64.cpp
    cpl_vsil_win32.cpp
    cpl_vsi_mem.cpp
    cpl_vsisimple.cpp)

add_library(cpl ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(cpl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

