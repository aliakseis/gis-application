#include "gisshpfilereader.h"

#include "shapelib/shapefil.h"

#include <cstring>

namespace {

/**
    * @brief Fill entity.points() with points from SHPObject.
    * @param entity - GisEntity class, for containing information about a
    * feature.
    * @param shpObject - SHPObject, that we get in openFile() function.
    */
void fillEntityWithPoints(GisEntity& entity, const SHPObject* shpObject) {
    for (int iVertexNumber = 0; iVertexNumber < shpObject->nVertices; ++iVertexNumber) {
        entity.addPoint(GAPoint(shpObject->padfX[iVertexNumber], shpObject->padfY[iVertexNumber]));
    }
}

/**
    * @brief Get iFieldNumber field from iRecordNumber entity from DBFHandle
    * file as string.
    * @param dbfFile - DBFHandle class that we get in openFile() function.
    * @param iFieldNumber - Number of field we want to get.
    * @param iRecordNumber - Index number of entity from file.
    * @return Field value as string.
    */
std::string getFieldValueAsString(const DBFHandle dbfFile, int iFieldNumber,
    int iRecordNumber) {
    return std::string(DBFReadStringAttribute(dbfFile, iRecordNumber, iFieldNumber));
}

/**
    * @brief Fill entity.fields() with fields from DBFHandle
    * @param dbfFile - DBFHandle class that we get in openFile() function.
    * @param entity - GisEntity class, for containing information about a
    * feature.
    * @param iEntityNumber - Index number of entity from file.
    */
void fillEntityWithFields(const DBFHandle dbfFile, GisEntity& entity,
    int iEntityNumber) {
    char pcFieldName[12];
    int iNumOfFields = DBFGetFieldCount(dbfFile);
    for (int iFieldNumber = 0; iFieldNumber < iNumOfFields; ++iFieldNumber) {
        // Get field name
        DBFGetFieldInfo(dbfFile, iFieldNumber, pcFieldName, nullptr, nullptr);
        // Get field value
        std::string sFieldValue = getFieldValueAsString(dbfFile, iFieldNumber, iEntityNumber);
        // Set name and value to entity
        entity.addField(GisField(std::string(pcFieldName), sFieldValue));
    }
}

} // namespace

GisShpFileReader::GisShpFileReader(const std::string& sFileName)
    : GisFileReader(sFileName), iShapeType_(0), iNumOfEntities_(0) {}

bool GisShpFileReader::readFile() {
    // Open files in readonly mode
    SHPInfo* shapeFile = SHPOpen(filename_.c_str(), "rb");
    DBFHandle dbfFile = DBFOpen(filename_.c_str(), "rb");

    if (!shapeFile || !dbfFile) {
        return false;
    }

    double pdMin[4];  // To get min coords {x, y, z, m}
    double pdMax[4];  // To get max coords {x, y, z, m}

    // Get some information to get data about entities
    SHPGetInfo(shapeFile, &iNumOfEntities_, &iShapeType_, pdMin, pdMax);

    minX_ = pdMin[0];
    minY_ = pdMin[1];
    maxX_ = pdMax[0];
    maxY_ = pdMax[1];

    entities_.clear();

    // For each entity fill structure GisEntity and put it to entities_
    for (int iEntityNumber = 0; iEntityNumber < iNumOfEntities_; ++iEntityNumber) {
        // Get entity
        SHPObject* shpObject = SHPReadObject(shapeFile, iEntityNumber);

        GisEntity newEntity;
        fillEntityWithPoints(newEntity, shpObject);
        fillEntityWithFields(dbfFile, newEntity, iEntityNumber);
        entities_.push_back(newEntity);

        SHPDestroyObject(shpObject);
    }

    DBFClose(dbfFile);
    SHPClose(shapeFile);

    return true;
}

// bool GisShpFileReader::readFile(const std::string& sFileName) {
//    filename_ = sFileName;
//    return readFile();
//}
