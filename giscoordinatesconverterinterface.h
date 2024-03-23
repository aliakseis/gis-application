#pragma once

class GAPoint;

class GisCoordinatesConverterInterface {
   public:
    virtual ~GisCoordinatesConverterInterface() = default;
    virtual GAPoint transformCoordinate(const GAPoint& sourceCoordinate) = 0;
    virtual GAPoint transformCoordinateBack(const GAPoint& sourceCoordinate) = 0;
};
