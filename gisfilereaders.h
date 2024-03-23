#pragma once

/**
  @file
  This file is contains function that take filename as parameter and abstract
  user from choosing suitable class for concrete file.\n Another function is
  used for parsing filename.
  */

#include <locale>
#include <string>
#include <vector>

#include "gisfilereader.h"
#include "gisshpfilereader.h"
#include "gistabfilereader.h"

enum GisFileType { GisUnknownType, GisTypeShp, GisTypeTab };

/**
 * @brief Function, that decide which instance to create based on filename.
 * @param filename - map filename that we want to open.
 * @return Pointer to GisFileReader that we use to operate with content of the
 * file.
 */
GisFileReader* gisCreateGisFileReader(const std::string& filename);

GisFileType getFileTypeByExtension(const std::string& filename);

std::string toLowerCase(const std::string& str);
