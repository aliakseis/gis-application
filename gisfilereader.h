#pragma once

/**
  @file
  This file contains declaration of abstract class GisFileReader.
  */

#include "gisentity.h"
#include "gisfield.h"

class GisFileReader {
   public:
    GisFileReader();
    GisFileReader(std::string filename);
    virtual ~GisFileReader();

    /**
     * @brief Must be implemented to fill GisEntity structure.
     * @return True - if file opened correctly. False - otherwise.
     */
    virtual bool readFile() = 0;

    /**
     * @brief Must be implemented to fill GisEntity structure.
     * @return True - if file opened correctly. False - otherwise.
     */
    virtual bool readFile(const std::string& filename);

    /**
     * @brief Get maximal X of whole map file.
     * @return Maximal X of whole map file.
     */
    double maxX() const;

    /**
     * @brief Get minimal X of whole map file.
     * @return Minimal X of whole map file.
     */
    double minX() const;

    /**
     * @brief Get maximal Y of whole map file.
     * @return Maximal Y of whole map file.
     */
    double maxY() const;

    /**
     * @brief Get minimal Y of whole map file.
     * @return Minimal Y of whole map file.
     */
    double minY() const;

    /**
     * @brief Get name of map file.
     * @return Filename of map file.
     */
    std::string filename() const;
    virtual void setFilename(const std::string& filename);

    /**
     * @brief Get list of GisEntity objects that contain information about
     * entities.
     * @return List of GisEntity.
     */
    const std::list<GisEntity>& entities() const;

    std::list<GisEntity>& entities();

    int entitiesPointsCount() const;

    void clipPolygons(double clipAreaLeft, double clipAreaTop, double clipAreaRight,
                      double clipAreaBottom);

    void restorePolygons();

   protected:
    std::list<GisEntity> entities_;
    std::list<GisEntity> entitiesClipBackup_;
    std::string filename_;
    double maxX_;
    double minX_;
    double maxY_;
    double minY_;
};
