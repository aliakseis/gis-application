
#include "gautils.h"


static const double MARGIN = 0.001;

bool GA::equal(double d1, double d2) {
    // We think that d1 and d2 are equal if numbers difference is less then 0.001
    return std::abs(d1 - d2) < MARGIN;
}

bool GA::less(double d1, double d2) {
    // if (equal(d1, d2)) {
    //     return false;
    // } else {
    //     return d1 < d2;
    // }
    return d1 <= d2 - MARGIN;
}

bool GA::greater(double d1, double d2) { return less(d2, d1); }

std::list<double> GA::calculateQuadraticEquation(double a, double b, double c) {
    std::list<double> listRoots;

    // find a discriminant
    double d = b * b - 4 * a * c;

    // a == 0: bx + C = 0
    if (equal(a, 0)) {
        // bx = - c => x = - c / b
        if (!equal(b, 0)) {
            listRoots.push_back(-c / b);
        } else {
            // if b == 0 - no roots
        }

    } else if (equal(d, 0)) {
        // only one root
        listRoots.push_back(-b / (2 * a));

    } else if (d > 0) {
        // if d > 0 number of roots is 2
        double r1 = (-b + sqrt(d)) / (2 * a);
        double r2 = (-b - sqrt(d)) / (2 * a);
        listRoots.push_back(r1);
        listRoots.push_back(r2);
    } else {
        // d < 0 - no roots
    }
    return listRoots;
}

double GA::getAngleByPoints(double x1, double y1, double x2, double y2) {
    // East is 0 degree, north is 90 degree  Number of degree is increasing in
    // conter-clockwise direction. X coordinate is increasing by moving to the
    // right. Y coordinate is increasing by moving to the up.
    // Сheck for boundary values
    if (equal(x1, x2) && equal(y1, y2)) {
        return 0;
    }
    if (equal(x1, x2)) {
        if (y1 < y2) {
            return 90;
        }
        if (y1 > y2) {
            return 270;
        }
    } else if (equal(y1, y2)) {
        if (x1 < x2) {
            return 0;
        }
        if (x1 > x2) {
            return 180;
        }
    } else {
        // Tangens is relation between two sides that form needed angle
        double x = x2 - x1;
        double y = y2 - y1;

        double tangens = y / x;
        // Take arctangens from tangens and get desired angle
        double angle = std::atan(tangens) * 180 / M_PI;

        // Arctangens can return value in range from 90 to -90. So in this case
        // we can't get angle on the left side of the coordinate plot. So, if x
        // is negative we know, that angle must be on the left side of the plot.
        // Thus we transform our "right-sided" angle to the "left-sided".
        if (x < 0) {
            angle += 180;
        }

        positiveAngle(angle);

        return angle;
    }
    return 0;
}

double GA::positiveAngle(double angle) {
    if (angle < 0) {
        do {
            angle += 360;
        } while (angle < 0);
    } else {
        while (360 < angle) {
            angle -= 360;
        }
    }

    return angle;
}

double GA::reverseAngle(double angle) {
    double revAngle = angle - 180;

    // Get positive angle on the same position on the trigonometric circle
    return positiveAngle(revAngle);
}

double GA::radians(double degree) { return degree * M_PI / 180; }

double GA::degree(double radians) { return radians * 180 / M_PI; }

double GA::vectorPolarAngle(double x, double y)  //, double length)
{
    double angleRadian = atan2(y, x);  // std::acos(x / length);
    double angleDegree = degree(angleRadian);

    // if (y < 0) {
    //     angleDegree = positiveAngle(-angleDegree);
    // }

    return angleDegree;
}

double GA::headingPolar(double angle) { return GA::positiveAngle(90 - angle); }

bool GA::isNumberBelong(double number, double min, double max) {
    double minActualValue = min;
    double maxActualValue = max;

    if (max < min) {
        minActualValue = max;
        maxActualValue = min;
    }

    return (minActualValue < number || equal(number, minActualValue)) &&
           (number < maxActualValue || equal(number, maxActualValue));
}

bool GA::isAnglePolarBetween(double angle, double angleFirst, double angleSecond) {
    if (equal(angle, angleFirst) || equal(angle, angleSecond)) {
        return true;
    }

    double minAngle = angleFirst;
    double maxAngle = angleSecond;

    if (angleSecond < angleFirst) {
        minAngle = angleSecond;
        maxAngle = angleFirst;
    }

    double diffAngles = std::abs(angleSecond - angleFirst);

    // The idea of the algorithm is to rotate all the corners
    // evenly until the smaller angle is aligned with zero.

    if (diffAngles < 180) {
        maxAngle = positiveAngle(maxAngle - minAngle);
        angle = positiveAngle(angle - minAngle);

        return angle < maxAngle;
    }
    if (180 < diffAngles) {
        double maxAngleDiffFromOrigin = 360 - maxAngle;
        minAngle = positiveAngle(minAngle + maxAngleDiffFromOrigin);
        angle = positiveAngle(angle + maxAngleDiffFromOrigin);

        return angle < minAngle;
    }
    return true;
}
