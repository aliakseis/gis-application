#include "gisfilereaders.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

GisFileReader* gisCreateGisFileReader(const std::string& filename) {
    // Parse string to know extension of the given file and then based on this
    // choose what object to create.

    GisFileReader* gisFileReader = nullptr;

    switch (getFileTypeByExtension(filename)) {
        case GisTypeShp:
            gisFileReader = new GisShpFileReader(filename);
            break;
        case GisTypeTab:
            gisFileReader = new GisTabFileReader(filename);
            break;
        default:
            break;
    }

    return gisFileReader;
}

GisFileType getFileTypeByExtension(const std::string& filename) {
    if (3 <= filename.size()) {
        std::string extension = filename.substr(filename.size() - 3, 3);
        std::string extensionLowerCase = toLowerCase(extension);

        if (extensionLowerCase == "shp") {
            return GisTypeShp;
        }
        if (extension == "tab") {
            return GisTypeTab;
        }
    }

    return GisUnknownType;
}

std::string toLowerCase(const std::string& str) {
    std::locale locale;
    std::string lowerString;

    for (char i : str) {
        lowerString += std::tolower(i, locale);
    }

    return lowerString;
}
