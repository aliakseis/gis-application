#pragma once

/**
  @file
  This file contains declaration of class GAVector.
  */

#include <functional>

#include "gapoint.h"

/**
 * @brief The GAVector class interprets geometrical vector.
 * @details This class has some uncommon and useful vector operations and
 * calculations, directed to perform necessary for this library calculations.
 */
class GAVector {
   public:

    /**
     * @brief Empty constructor without any initializations.
     */
    GAVector();

    /**
     * @brief Constructor, that initializes GAVector::point(), hence
     * GAVector::x() and GAVector::y(). And normalizes it.
     * @param x - x coord of the vector
     * @param y - y coord of the vector
     * @param normalizeVector - if True vector will be normalized,
     * if False vector will be not normalized.
     */
    GAVector(double x, double y, bool normalizeVector = true);

    /**
     * @brief Constructor, that initializes GAVector::point(), hence
     * GAVector::x() and GAVector::y(). And normalizes it.
     * @param point - point of the vector
     */
    GAVector(const GAPoint& point);

    /**
     * @brief Constructor, that initializes GAVector::point(), hence
     * GAVector::x() and GAVector::y(). And normalizes it.
     * @details Converts vector defined by two point into vector defined by one
     * point and normalizes it.
     * @param pointBegin - point of the begin of the vector
     * @param pointEnd - point of the end of the vector
     */
    GAVector(const GAPoint& pointBegin, const GAPoint& pointEnd, bool normalizeVector = true);

    GAVector(double anglePolar);

    static GAVector fromHeadingAngle(double headingAngle);

    /**
     * @brief Get x coordinate of the vector.
     * @return x coordinate of the vector.
     */
    double x() const;

    /**
     * @brief Get y coordinate of the vector.
     * @return y coordinate of the vector.
     */
    double y() const;

    /**
     * @brief Whether equal two GAVector instances
     * @param otherVector - second GAVector instance
     * @return True - if vectors are equal to each other. False - otherwise.
     */
    bool operator==(const GAVector& otherVector) const;

    /**
     * @brief Get point of the vector.
     * @return point of the vector.
     */
    GAPoint point() const;

    /**
     * @brief Set point of the vector.
     * @param point - point of the vector.
     */
    void setPoint(const GAPoint& point);

    /**
     * @brief Get vector, that is perpendicular to this vector.
     * @return Calculated vector.
     */
    GAVector perpendicularVector() const;

    /**
     * @brief Get vector, that is perpendicular to this vector.
     * The calculation of the normalized vector.
     * @return Calculated vector.
     */
    GAVector perpendicularNormalVector() const;

    /**
     * @brief Get vector with reversed direction.
     * @return Calculated vector.
     */
    GAVector reverseVector() const;

    /**
     * @brief Normalizes the vector.
     */
    void normalize();

    /**
     * @brief Is vector's point initialized or not.
     * @return True - if vector's point is initialized. False - otherwise.
     */
    bool isEmpty() const;

    /**
     * @brief Is the vector a zero vector.
     * @return True - if the vector is a zero vector. False - otherwise.
     */
    bool isNull() const;

    /**
     * @brief Get angle between the vectors.
     * @param otherVector - another vector.
     * @return angle between the vectors.
     */
    double angle(const GAVector& otherVector) const;

    /**
     * @brief Give polar angle of this vector in degrees.
     * @return Angle in degrees.
     */
    double anglePolar() const;

    /**
     * @brief Give heading angle of this vector in degrees.
     * @return Angle in degrees.
     */
    double angleHeading() const;

    friend std::ostream& operator<<(std::ostream& os, const GAVector& vector);

   private:
    GAPoint point_;
};
