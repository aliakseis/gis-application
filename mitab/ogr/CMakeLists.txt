set(HEADER_FILES
    ogr_api.h
    ogr_attrind.h
    ogr_core.h
    ogr_feature.h
    ogr_featurestyle.h
    ogr_gensql.h
    ogr_geometry.h
    ogr_geos.h
    ogr_p.h
    ogrsf_frmts.h
    ogr_spatialref.h
    ogr_srs_api.h
    swq.h)

set(SOURCE_FILES
    swq.cpp
    gml2ogrgeometry.cpp
    ogr2gmlgeometry.cpp
    ogr_api.cpp
    ogr_attrind.cpp
#    ogrct.cpp
    ogrcurve.cpp
    ogrdatasource.cpp
    ogrfeature.cpp
    ogrfeaturedefn.cpp
    ogrfeaturequery.cpp
    ogrfeaturestyle.cpp
    ogrfielddefn.cpp
    ogr_fromepsg.cpp
    ogr_gensql.cpp
    ogrgeometrycollection.cpp
    ogrgeometry.cpp
    ogrgeometryfactory.cpp
    ogrlayer.cpp
    ogrlinearring.cpp
    ogrlinestring.cpp
    ogr_miattrind.cpp
    ogrmultilinestring.cpp
    ogrmultipoint.cpp
    ogrmultipolygon.cpp
    ogrpoint.cpp
    ogrpolygon.cpp
    ogrsfdriver.cpp
    ogrsfdriverregistrar.cpp
    ogrspatialreference.cpp
    ogr_srs_dict.cpp
    ogr_srs_esri.cpp
    ogr_srsnode.cpp
    ogr_srs_proj4.cpp
    ogr_srs_xml.cpp
    ogrsurface.cpp
    ogrutils.cpp)

add_library(ogr ${HEADER_FILES} ${SOURCE_FILES})
target_compile_definitions(ogr PUBLIC "PROJ_STATIC")
target_link_libraries(ogr PRIVATE cpl mitab)
target_include_directories(ogr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../cpl ${CMAKE_CURRENT_SOURCE_DIR}/../mitab)


