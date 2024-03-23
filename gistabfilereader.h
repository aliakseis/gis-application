#pragma once

/**
  @file
  This file contains declaration of class GisTabFileReader.
  */

#include <string>

#include "gisfilereader.h"

class IMapInfoFile;

/**
 * @brief Class that allows you to get information from .tab file.
 */
class GisTabFileReader : public GisFileReader {
   public:
    GisTabFileReader(const std::string& filename);

    /**
     * @brief Opens file by filename that was passed into constructor.
     * @return True - if file was opened successfully. False - otherwise.
     */
    virtual bool readFile();

   private:
    /**
     * @brief Initializes maxX_, maxY_, minX_, minY_ which derived from
     * GisFileReader.
     */
    void fillLimitsCoordinates();

    /**
     * @brief Fill GisFileReader::entities() with points and fields by data from
     * mapInfoFile_.
     */
    void fillEntities();

    IMapInfoFile* mapInfoFile_;
};
