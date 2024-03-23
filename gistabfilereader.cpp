#include "gistabfilereader.h"

#include "mitab.h"

#include <set>

namespace {

    /**
     * @brief Removes whispaces in the begining of given string.
     * @param string - string where we want to remove prior whitespaces.
     * @return Cleared string.
     */
std::string& clearFromWhitespaces(std::string& s) {
    auto lam = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), lam));
    s.erase(std::find_if(s.rbegin(), s.rend(), lam).base(), s.end());
    return s;
}


    /**
     * @brief Get list of fields from feature.
     * @param feature - OGRFeature from which we get fields.
     * @return List of fields from feature.
     */
std::list<GisField> featureFields(OGRFeature* feature) {
    std::list<GisField> fields;

    for (int i = 0; i < feature->GetFieldCount(); i++) {
        std::string fieldName = feature->GetFieldDefnRef(i)->GetNameRef();
        OGRFieldType fieldType = feature->GetFieldDefnRef(i)->GetType();

        std::string fieldValue;

        // Translate to needed data type from given
        switch (fieldType) {
            case OFTInteger:
                fieldValue = std::to_string(feature->GetFieldAsInteger(i));
                break;
            case OFTString:
                fieldValue = feature->GetFieldAsString(i);
                break;
            default:
                break;
        }

        fields.emplace_back(fieldName, clearFromWhitespaces(fieldValue));
    }

    return fields;
}

    /**
     * @brief Get list of points from feature.
     * @param feature - OGRFeature from which we get fields.
     * @return List of points from feature.
     */
std::list<GAPoint> featurePoints(OGRFeature* feature) {
    std::list<GAPoint> points;

    OGRGeometry* geometry = feature->GetGeometryRef();

    // If entity is polygon then cast it to OGRPolygon type and eject points.
    if (geometry->getGeometryType() == wkbPolygon) {
        auto* polygon = static_cast<OGRPolygon*>(geometry);

        OGRLinearRing* linearRing = polygon->getExteriorRing();

        for (int indexPoint = 0; indexPoint < linearRing->getNumPoints(); indexPoint++) {
            points.emplace_back(linearRing->getX(indexPoint), linearRing->getY(indexPoint));
        }
    }

    return points;
}

} // namespace

GisTabFileReader::GisTabFileReader(const std::string& filename)
    : GisFileReader(filename), mapInfoFile_(nullptr) {}

bool GisTabFileReader::readFile() {

    delete mapInfoFile_;

    mapInfoFile_ = IMapInfoFile::SmartOpen(filename_.c_str());

    if (mapInfoFile_) {
        entities_.clear();

        fillLimitsCoordinates();
        fillEntities();

        return true;
    }

    return false;
}

void GisTabFileReader::fillLimitsCoordinates() {
    // Find min and max x and y from all boundaries from the map.

    std::set<double> xValues;
    std::set<double> yValues;

    for (int i = 1; i <= mapInfoFile_->GetFeatureCount(1); i++) {
        TABFeature* feature = mapInfoFile_->GetFeatureRef(i);
        double minX;
        double maxX;
        double minY;
        double maxY;
        feature->GetMBR(minX, minY, maxX, maxY);

        xValues.insert(minX);
        yValues.insert(minY);
    }

    minX_ = *xValues.begin();
    maxX_ = *xValues.rbegin();
    minY_ = *yValues.begin();
    maxY_ = *yValues.rbegin();
}

void GisTabFileReader::fillEntities() {
    // Return to the begin of the file
    mapInfoFile_->ResetReading();

    // Move around all features, fill GisEntity structure and add it to
    // entities_.
    while (OGRFeature* feature = mapInfoFile_->GetNextFeature()) {
        std::list<GisField> fields = featureFields(feature);
        std::list<GAPoint> points = featurePoints(feature);
        entities_.emplace_back(fields, points);
    }
}

