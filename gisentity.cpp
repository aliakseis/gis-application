#include "gisentity.h"

GisEntity::GisEntity() = default;

GisEntity::GisEntity(const std::string& field) { fields_.emplace_back("Field", field); }

GisEntity::GisEntity(const std::string& fieldName, const std::string& fieldValue) {
    fields_.emplace_back(fieldName, fieldValue);
}

GisEntity::GisEntity(std::list<GisField>& fieldsInit, std::list<GAPoint>& pointsInit) {
    fields_.splice(fields_.begin(), fieldsInit);
    points_.splice(points_.begin(), pointsInit);
}

bool GisEntity::isPointsEmpty() const { return points_.empty(); }

bool GisEntity::isFieldsEmpty() const { return fields_.empty(); }

const std::list<GisField>& GisEntity::fields() const { return fields_; }

std::string GisEntity::fieldsToString() const {
    std::string fieldsAsString;

    if (!fields_.empty()) {
        for (const auto& field : fields_) {
            fieldsAsString += field.name() + ":" + field.value() + ",";
        }
        fieldsAsString.erase(fieldsAsString.size() - 1, 1);
    }

    return fieldsAsString;
}

std::string GisEntity::entityInfo() const {
    std::string sizeString = std::to_string(points_.size());

    return fieldsToString() + " points_count:" + sizeString;
}

const std::list<GAPoint>& GisEntity::points() const { return points_; }

std::list<GAPoint>& GisEntity::points() { return points_; }

void GisEntity::addField(const GisField& field) { fields_.push_back(field); }

void GisEntity::addPoint(const GAPoint& point) { points_.push_back(point); }

GisEntity GisEntity::cloneWithoutPoints() const {
    GisEntity entity;
    entity.fields_ = fields_;

    return entity;
}
