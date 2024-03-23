#pragma once

/**
  @file
  This file contains namespace with functions for inner calculations and
  translating.
  */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>

/**
 * @brief Namespace with functions for inner calculations and translating.
 */
namespace GA {

/**
 * @brief Compare d1 and d2 values and return whether they equal to each other.
 * @param d1 - variable to compare with
 * @param d2 - variable to compare with
 * @return True - if equal, false - otherwise.
 */
bool equal(double d1, double d2);

bool less(double d1, double d2);
bool greater(double d1, double d2);

/**
 * @brief Calculate quadratic equation and return list of resulting roots.
 * @details Parameters could have any value. If value will be incorrect for
 * rules of quadratic equation then you will just get an empty list of roots
 * @param a - corresponds to "a" in formula of the general quadratic equation.
 * @param b - corresponds to "b" in formula of the general quadratic equation.
 * @param c - corresponds to "c" in formula of the general quadratic equation.
 * @return List of resulting roots.
 */
std::list<double> calculateQuadraticEquation(double a, double b, double c);

/**
 * @warning Not part of library. Used only for rotating picture in demo.
 * @brief Calculate degree by directional vector from first point to second.
 * @details East is 0 degree, north is 90 degree and so on. Number of degree is
 * increasing in conter-clockwise direction. \n X coordinate is increasing by
 * moving to the right. \n Y coordinate is increasing by moving to the up.
 * @param x1 - X coord of first point.
 * @param y1 - Y coord of first point.
 * @param x2 - X coord of second point.
 * @param y2 - Y coord of second point.
 * @return Number of degree. \n
 * Returns 0 if points are equal to each other.
 */
double getAngleByPoints(double x1, double y1, double x2, double y2);

/**
 * @brief Converts degree to radians.
 * @param degree - value of degree to convert to radians.
 * @return Radians value.
 */
double radians(double degree);

/**
 * @brief Converts radians to degree.
 * @param radians - value of radians to convert to degree.
 * @return Degree value.
 */
double degree(double radians);

/**
 * @brief Converts negative value of angle to it's positive eqivalent.
 * @details Does nothing if angle is positive.
 * @param angle - negative angle positive equivalent of which we want to get.
 * @return Positive angle.
 */
double positiveAngle(double angle);

/**
 * @brief Get angle, that placed on a trigonometric circle on the opposite side
 * from passed angle.
 * @details In other words:\n
 * reverseAngle = angle - 180\n
 * or\n
 * reverseAngle = angle + 180
 * @param angle - current angle value, that we want to reverse.
 * @return Reversed angle value.
 */
double reverseAngle(double angle);

/**
 * @brief Get polar angle from coordinates of normal vector.
 * @param x X-Coordinate of vector.
 * @param y Y-Coordinate of vector.
 * @param length Legth of vector.
 * @return Result angle in degrees.
 */
double vectorPolarAngle(double x, double y);  // , double length = 1);

/**
 * @brief Converts heading angle in degrees to polar angle and back.
 * @param angle Angle (polar or heading) in degrees.
 * @return angle (heading or polar) in degrees.
 */
double headingPolar(double angle);

/**
 * @brief Checks if number belongs to interval.
 * @param min - Min interval value.
 * @param max - Max interval value.
 * @return @b true - if number belongs to interval, @b false otherwise.
 */
bool isNumberBelong(double number, double min, double max);

bool isAnglePolarBetween(double angle, double angleFirst, double angleSecond);

}  // namespace GA
