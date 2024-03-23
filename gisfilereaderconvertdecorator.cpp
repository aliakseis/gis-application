#include "gisfilereaderconvertdecorator.h"

GisFileReaderConvertDecorator::GisFileReaderConvertDecorator()
    : gisFileReader_(nullptr), coordinatesConverter_(nullptr) {}

GisFileReaderConvertDecorator::GisFileReaderConvertDecorator(
    GisFileReader *gisFileReader, GisCoordinatesConverterInterface *coordinatesConverter)
    : gisFileReader_(gisFileReader), coordinatesConverter_(coordinatesConverter) {}

GisFileReaderConvertDecorator::~GisFileReaderConvertDecorator() {

    delete gisFileReader_;

    delete coordinatesConverter_;
}

bool GisFileReaderConvertDecorator::readFile() {
    if (!gisFileReader_ || !coordinatesConverter_) {
        return false;
    }

    bool openFileResult = gisFileReader_->readFile();

    if (!openFileResult) {
        return false;
    }

    entities_.clear();
    fillDecoratorEntities();

    return true;
}

bool GisFileReaderConvertDecorator::readFile(const std::string &filename) {
    gisFileReader_->setFilename(filename);
    return readFile();
}

void GisFileReaderConvertDecorator::fillDecoratorEntities() {
    std::set<double> xValues;
    std::set<double> yValues;

    for (const auto &entityIter : gisFileReader_->entities()) {
        entities_.emplace_back();

        for (const auto &pointsIter : entityIter.points()) {
            GAPoint pointConverted = coordinatesConverter_->transformCoordinate(pointsIter);

            entities_.back().addPoint(pointConverted);

            xValues.insert(pointConverted.x());
            yValues.insert(pointConverted.y());
        }

        for (const auto &fieldsIter : entityIter.fields()) {
            entities_.back().addField(fieldsIter);
        }
    }

    minX_ = *xValues.begin();
    maxX_ = *xValues.rbegin();
    minY_ = *yValues.begin();
    maxY_ = *yValues.rbegin();
}

GisFileReader *GisFileReaderConvertDecorator::gisFileReader() { return gisFileReader_; }

void GisFileReaderConvertDecorator::setGisFileReader(GisFileReader *gisFileReader) {

    delete gisFileReader_;

    gisFileReader_ = gisFileReader;
}

void GisFileReaderConvertDecorator::setFilename(const std::string &filename) {
    GisFileReader::setFilename(filename);
    gisFileReader_->setFilename(filename);
}

GisCoordinatesConverterInterface *GisFileReaderConvertDecorator::coordinatesConverter() {
    return coordinatesConverter_;
}

void GisFileReaderConvertDecorator::setCoordinatesConverter(
    GisCoordinatesConverterInterface *coordinatesConverter) {

    delete coordinatesConverter_;

    coordinatesConverter_ = coordinatesConverter;
}
