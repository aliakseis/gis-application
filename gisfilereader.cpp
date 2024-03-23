#include "gisfilereader.h"

#include "clipper.hpp"

#include <utility>


namespace {

const int precision = 100;

ClipperLib::Path pathFromRectangle(double clipAreaLeft, double clipAreaTop,
                                                  double clipAreaRight, double clipAreaBottom) {

    long long left = std::llround(clipAreaLeft * precision);
    long long top = std::llround(clipAreaTop * precision);
    long long right = std::llround(clipAreaRight * precision);
    long long bottom = std::llround(clipAreaBottom * precision);

    ClipperLib::Path path;
    path.push_back(ClipperLib::IntPoint(left, top));
    path.push_back(ClipperLib::IntPoint(right, top));
    path.push_back(ClipperLib::IntPoint(right, bottom));
    path.push_back(ClipperLib::IntPoint(left, bottom));

    return path;
}

void fillPathFromEntity(ClipperLib::Path &path, const GisEntity &entity) {
    for (const auto &pointEntity : entity.points()) {

        path.push_back(ClipperLib::IntPoint(std::llround(pointEntity.x() * precision),
                                            std::llround(pointEntity.y() * precision)));
    }
}

void fillEntityFromPath(GisEntity &entity, const ClipperLib::Path &path) {
    for (auto point : path) {
        double x = static_cast<double>(point.X) / precision;
        double y = static_cast<double>(point.Y) / precision;
        entity.addPoint(GAPoint(x, y));
    }
}

} // namespace

GisFileReader::GisFileReader() = default;

GisFileReader::GisFileReader(std::string filename) : filename_(std::move(filename)) {}

GisFileReader::~GisFileReader() = default;

bool GisFileReader::readFile(const std::string &filename) {
    filename_ = filename;
    return readFile();
}

double GisFileReader::maxX() const { return maxX_; }

double GisFileReader::minX() const { return minX_; }

double GisFileReader::maxY() const { return maxY_; }

double GisFileReader::minY() const { return minY_; }

std::string GisFileReader::filename() const { return filename_; }

void GisFileReader::setFilename(const std::string &filename) { filename_ = filename; }

const std::list<GisEntity> &GisFileReader::entities() const { return entities_; }

std::list<GisEntity> &GisFileReader::entities() { return entities_; }

int GisFileReader::entitiesPointsCount() const {
    int pointsCount = 0;
    for (const auto &entitie : entities_) {
        pointsCount += entitie.points().size();
    }

    return pointsCount;
}

void GisFileReader::clipPolygons(double clipAreaLeft, double clipAreaTop, double clipAreaRight,
                                 double clipAreaBottom) {
    entitiesClipBackup_.clear();
    entitiesClipBackup_.splice(entitiesClipBackup_.begin(), entities_);

    ClipperLib::Path clipArea =
        pathFromRectangle(clipAreaLeft, clipAreaTop, clipAreaRight, clipAreaBottom);

    for (const auto &entity : entitiesClipBackup_)  {
        ClipperLib::Path pointsSource;
        fillPathFromEntity(pointsSource, entity);

        ClipperLib::Clipper clipper;
        clipper.AddPath(pointsSource, ClipperLib::ptSubject, true);
        clipper.AddPath(clipArea, ClipperLib::ptClip, true);

        ClipperLib::Paths clippedArea;
        clipper.Execute(ClipperLib::ctIntersection, clippedArea);

        for (auto &path : clippedArea) {
            entities_.push_back(entity.cloneWithoutPoints());
            fillEntityFromPath(entities_.back(), path);
        }
    }
}

void GisFileReader::restorePolygons() {
    if (entitiesClipBackup_.empty()) {
        return;
    }
    entities_.clear();
    entities_.splice(entities_.begin(), entitiesClipBackup_);
}
