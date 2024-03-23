set(HEADER_FILES
    mitab_capi.h
    mitab_geometry.h
    mitab.h
    mitab_ogr_driver.h
    mitab_priv.h
    mitab_utils.h)

set(SOURCE_FILES
    mitab_bounds.cpp
    mitab_capi.cpp
    mitab_coordsys.cpp
    mitab_datfile.cpp
    mitab_feature.cpp
    mitab_feature_mif.cpp
    mitab_geometry.cpp
    mitab_idfile.cpp
    mitab_imapinfofile.cpp
    mitab_indfile.cpp
    mitab_mapcoordblock.cpp
    mitab_mapfile.cpp
    mitab_mapheaderblock.cpp
    mitab_mapindexblock.cpp
    mitab_mapobjectblock.cpp
    mitab_maptoolblock.cpp
    mitab_middatafile.cpp
    mitab_miffile.cpp
    mitab_ogr_datasource.cpp
    mitab_ogr_driver.cpp
    mitab_rawbinblock.cpp
    mitab_spatialref.cpp
    mitab_tabfile.cpp
    mitab_tabseamless.cpp
    mitab_tabview.cpp
    mitab_tooldef.cpp
    mitab_utils.cpp)

add_library(mitab ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(mitab PRIVATE ogr cpl)
target_include_directories(mitab PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


