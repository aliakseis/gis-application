#pragma once

/**
  @file
  This file contains declaration of class GAPoint.
  */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>


class GAVector;
class GAPoint;


/**
 * @brief The GAPoint class interprets point and some functions for calculations
 * over the point.
 */
class GAPoint {
   public:
    /**
     * @brief Constructor that initializes defaults: x() = 0, y() = 0,
     * isEmpty(). = true.
     */
    GAPoint();
    /**
     * @brief Constructor that allow user to initialize  x(), y(), isEmpty().
     * @param x - to set x().
     * @param y - to set y().
     * @param isEmpty - to set isEmpty().
     */
    GAPoint(double x, double y, bool isEmpty = false);

    /**
     * @brief Copy constructor.
     * @param point - GAPoint instance to copy from.
     */
    GAPoint(const GAPoint& point);

    /**
     * @brief operator =
     * @param otherPoint - GAPoint instance to copy from.
     * @return The point after change.
     */
    GAPoint& operator=(const GAPoint& otherPoint);

    bool operator==(const GAPoint& otherPoint) const;
    bool operator!=(const GAPoint& otherPoint) const;

    /**
     * @brief Get value of x coordinate of the point.
     * @return x coordinate of the point.
     */
    double x() const;

    /**
     * @brief Set value of x coordinate of the point.
     * @return x coordinate of the point.
     */
    void setX(double x);

    /**
     * @brief Get value of y coordinate of the point.
     * @return y coordinate of the point.
     */
    double y() const;

    /**
     * @brief Set value of y coordinate of the point.
     * @return y coordinate of the point.
     */
    void setY(double y);

    /**
     * @brief Get isEmpty flag.
     * @return True - if user set whether x() coord or y() coord in constructor
     * or manually. False - otherwise.
     */
    bool isEmpty() const;

    /**
     * @brief Set isEmpty() flag.
     * @param isEmpty - value to set.
     */
    void setIsEmpty(bool isEmpty);

    /**
     * @brief Shift x() and y() coords by offsetX and offsetY.
     * @param offsetX - value to offset along the x axis.
     * @param offsetY - value to offset along the y axis.
     */
    void setOffset(double offsetX, double offsetY);

    /**
     * @brief Distance between the point and otherPoint.
     * @param otherPoint - point to know the distance.
     * @return Distance between points.
     */
    double distance(const GAPoint& otherPoint) const;

    friend std::ostream& operator<<(std::ostream& os, const GAPoint& point);

   private:
    bool isEmpty_;
    double x_;
    double y_;
};
