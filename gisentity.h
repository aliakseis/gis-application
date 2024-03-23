#pragma once

/**
  @file
  This file contains declaration of class GisEntity.
  */

#include <list>

#include "gapoint.h"
#include "gisfield.h"

/**
 * @brief Stores info about entity (feature) from gis files.
 */
class GisEntity {
   public:
    /**
     * @brief Empty constructor without any initializations.
     */
    GisEntity();

    GisEntity(const std::string& field);

    GisEntity(const std::string& fieldName, const std::string& fieldValue);

    /**
     * @brief Constructor with initialization GisEntity::fields() and
     * GisEntity::points().
     * @param fieldsInit - to initialize GisEntity::fields().
     * @param pointsInit - to initialize GisEntity::points().
     */
    GisEntity(std::list<GisField>& fieldsInit, std::list<GAPoint>& pointsInit);

    /**
     * @brief Is GisEntity::points() empty.
     * @return True - if GisEntity::points() is empty. False - otherwise.
     */
    bool isPointsEmpty() const;

    /**
     * @brief Is GisEntity::fields() empty.
     * @return True - if GisEntity::fields() is empty. False - otherwise.
     */
    bool isFieldsEmpty() const;

    /**
     * @brief Get list of fields.
     * @return List of fields.
     */
    const std::list<GisField>& fields() const;

    std::string fieldsToString() const;

    std::string entityInfo() const;

    /**
     * @brief Get list of points.
     * @return List of points.
     */
    const std::list<GAPoint>& points() const;
    std::list<GAPoint>& points();

    /**
     * @brief Adds new field into GisEntity::fields().
     * @param field	- field to add.
     */
    void addField(const GisField& field);

    /**
     * @brief Adds new point into GisEntity::points().
     * @param point	- point to add.
     */
    void addPoint(const GAPoint& point);

    GisEntity cloneWithoutPoints() const;

   private:
    std::list<GisField> fields_;
    std::list<GAPoint> points_;
};
