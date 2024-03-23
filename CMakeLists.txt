cmake_minimum_required(VERSION 3.16)
 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")

add_definitions(-D_USE_MATH_DEFINES)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(gis-application)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5
             COMPONENTS
             Gui
             Core
             Widgets
             Svg
             REQUIRED)

set(RESOURCES_FILES resources.qrc)

set(HEADER_FILES
    gapoint.h
    gavector.h
    gautils.h
    giscoordinatesconverterinterface.h
    giscoordinatesconvertersimple.h
    gisentity.h
    gisfield.h
    gisfilereaderconvertdecorator.h
    gisfilereader.h
    gisfilereaders.h
    gisshpfilereader.h
    gistabfilereader.h
    mainwidget.h
)


set(SOURCE_FILES
    gapoint.cpp
    gavector.cpp
    gautils.cpp
    giscoordinatesconvertersimple.cpp
    gisentity.cpp
    gisfield.cpp
    gisfilereaderconvertdecorator.cpp
    gisfilereader.cpp
    gisfilereaders.cpp
    gisshpfilereader.cpp
    gistabfilereader.cpp
)

set(UI_FILES mainwidget.ui)

add_executable(gis-application ${HEADER_FILES} ${SOURCE_FILES} main.cpp mainwidget.cpp ${RESOURCES_FILES} ${UI_FILES})
target_link_libraries(gis-application PUBLIC Qt5::Widgets Qt5::Svg clipper coordConvert cpl mitab ogr shapelib)


add_subdirectory(clipper)
add_subdirectory(coordConvert)
add_subdirectory(mitab)
add_subdirectory(shapelib)
