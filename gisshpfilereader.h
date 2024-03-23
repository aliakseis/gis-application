#pragma once

/**
  @file
  This file contains declaration of class GisShpFileReader.
  */

#include <string>

#include "gisfilereader.h"

/**
 * @brief Class that allows you to get information from .shp file.
 */
class GisShpFileReader : public GisFileReader {
   public:
    GisShpFileReader(const std::string& sFileName);

    /**
     * @brief Opens file by filename that was passed into constructor.
     * @return True - if file was opened successfully. False - otherwise.
     */
    virtual bool readFile();

   private:
    int iShapeType_;
    int iNumOfEntities_;
};
