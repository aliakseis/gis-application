#pragma once

#include <set>

#include "giscoordinatesconverterinterface.h"
#include "gisfilereader.h"

class GisFileReaderConvertDecorator : public GisFileReader {
   public:
    GisFileReaderConvertDecorator();
    GisFileReaderConvertDecorator(GisFileReader* gisFileReader,
                                  GisCoordinatesConverterInterface* coordinatesConverter);

    virtual ~GisFileReaderConvertDecorator();

    virtual bool readFile();
    virtual bool readFile(const std::string& filename);

    GisCoordinatesConverterInterface* coordinatesConverter();
    void setCoordinatesConverter(GisCoordinatesConverterInterface* coordinatesConverter);

    GisFileReader* gisFileReader();
    void setGisFileReader(GisFileReader* gisFileReader);

    virtual void setFilename(const std::string& filename);

   private:
    void fillDecoratorEntities();

    GisFileReader* gisFileReader_;
    GisCoordinatesConverterInterface* coordinatesConverter_;
};
