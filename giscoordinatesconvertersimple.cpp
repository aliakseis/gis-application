#include "giscoordinatesconvertersimple.h"

#include "coordconvert.h"

#include "gapoint.h"
#include "gautils.h"

GisCoordinatesConverterSimple::GisCoordinatesConverterSimple(double centerLongitude,
                                                             double centerLatitude) {
    Init_CoordinateTransformation(GA::radians(centerLongitude), GA::radians(centerLatitude), 0);
}

GisCoordinatesConverterSimple::~GisCoordinatesConverterSimple() = default;

GAPoint GisCoordinatesConverterSimple::transformCoordinate(const GAPoint &sourceCoordinate) {
    INERTIAL_POSITION outputCoordinate;
    Coordinate_Transform_LongLat_To_ROCInner(
        GA::radians(sourceCoordinate.x()), GA::radians(sourceCoordinate.y()), 0, &outputCoordinate);

    return {outputCoordinate.Inertial_X_f, outputCoordinate.Inertial_Y_f};
}

GAPoint GisCoordinatesConverterSimple::transformCoordinateBack(const GAPoint &sourceCoordinate) {
    INERTIAL_POSITION sourceCoordinateStruct;
    sourceCoordinateStruct.Inertial_X_f = sourceCoordinate.x();
    sourceCoordinateStruct.Inertial_Y_f = sourceCoordinate.y();
    sourceCoordinateStruct.Inertial_Z_f = 0;

    double longitudeRadians;
    double latitudeRadians;
    double altitude;
    double radius;

    Coordinate_Transform_ROCInner_To_LongLat(&sourceCoordinateStruct, &longitudeRadians,
                                             &latitudeRadians, &altitude, &radius);

    return {GA::degree(longitudeRadians), GA::degree(latitudeRadians)};
}
