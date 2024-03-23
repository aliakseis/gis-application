#pragma once

/**
  @file
  This file contains declaration of class GisField.
  */

#include <string>

/**
 * @brief Stores info about field from gis files.
 */
class GisField {
   public:
    /**
     * @brief Initialize with name of field and it's value.
     * @param name - name of the field.
     * @param value - value of the field.
     */
    GisField(std::string name, std::string value);

    /**
     * @brief Get name of the field.
     * @return name of the vield.
     */
    std::string name() const;

    /**
     * @brief Set name of the field.
     */
    void setName(const std::string& name);

    /**
     * @brief Get value of the field.
     * @return value of the vield as string.
     */
    std::string value() const;

    /**
     * @brief Set value of the field.
     */
    void setValue(const std::string& value);

    /**
     * @brief Get value of the field as integer.
     * @warning Not implemented.
     * @return value of the field as integer.
     */
    int valueAsInt() const;

    /**
     * @brief Get value of the field as double.
     * @warning Not implemented.
     * @return value of the field as double.
     */
    double valueAsDouble() const;

    /**
     * @brief Get value of the field as std::string.
     * @warning Not implemented.
     * @return value of the field as std::string.
     */
    std::string valueAsString() const;

   private:
    std::string name_;
    std::string value_;
};
