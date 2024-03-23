#include "gisfield.h"

#include <utility>

GisField::GisField(std::string name, std::string value)
    : name_(std::move(name)), value_(std::move(value)) {}

std::string GisField::name() const { return name_; }

void GisField::setName(const std::string &name) { name_ = name; }

std::string GisField::value() const { return value_; }

void GisField::setValue(const std::string &value) { value_ = value; }
