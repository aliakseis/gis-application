set(HEADER_FILES
    shapefil.h)

set(SOURCE_FILES
    dbfopen.c
    safileio.c
    sbnsearch.c
    shpopen.c
    shptree.c)

add_library(shapelib ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(shapelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
