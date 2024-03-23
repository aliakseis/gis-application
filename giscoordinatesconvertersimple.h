#pragma once

#include "giscoordinatesconverterinterface.h"

class GisCoordinatesConverterSimple : public GisCoordinatesConverterInterface {
   public:
    GisCoordinatesConverterSimple(double mapCenterLongitude, double mapCenterLatitude);
    virtual ~GisCoordinatesConverterSimple();

    virtual GAPoint transformCoordinate(const GAPoint& sourceCoordinate);
    virtual GAPoint transformCoordinateBack(const GAPoint& sourceCoordinate);
};
