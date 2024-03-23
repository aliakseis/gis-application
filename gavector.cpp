#include "gavector.h"

#include "gautils.h"

GAVector::GAVector() = default;

GAVector::GAVector(double x, double y, bool normalizeVector) : point_(x, y) {
    // It is more convenient to carry out calculations using a unit length
    // vector, therefore, we normalize the vector.
    if (normalizeVector) {
        normalize();
    }
}

GAVector::GAVector(const GAPoint &point)
    // Constructor delegation is used.
    : GAVector(point.x(), point.y()) {
}

GAVector::GAVector(const GAPoint &pointBegin, const GAPoint &pointEnd, bool normalizeVector)
    // Constructor delegation is used.
    : GAVector(pointEnd.x() - pointBegin.x(), pointEnd.y() - pointBegin.y(), normalizeVector) {
}

GAVector::GAVector(double anglePolar) {
    double anglePolarRadian = GA::radians(anglePolar);
    point_.setX(std::cos(anglePolarRadian));
    point_.setY(std::sin(anglePolarRadian));
}

GAVector GAVector::fromHeadingAngle(double headingAngle) {
    double anglePolar = GA::headingPolar(headingAngle);
    return {anglePolar};
}

double GAVector::x() const
// The vector is determined by the coordinate of the point of its end (it is
// assumed that the beginning of the vector coincides with the origin).
{
    return point_.x();
}

double GAVector::y() const
// The vector is determined by the coordinate of the point of its end (it is
// assumed that the beginning of the vector coincides with the origin).
{
    return point_.y();
}

bool GAVector::operator==(const GAVector &otherVector) const {
    // If the coordinates of the points that define the vectors coincide, then
    // the vectors also coincide and are equal.
    return point_ == otherVector.point_;
}

GAPoint GAVector::point() const { return point_; }

void GAVector::setPoint(const GAPoint &point) {
    point_ = point;
    // To exclude the transfer of the coordinates of a point that corresponds to
    // a non-normalized vector, you need to normalize the vector.
    normalize();
}

GAVector GAVector::perpendicularVector() const {
    // Let the vector B(Bx, By) be the vector perpendicular to the current.
    // The scalar product of perpendicular winds is zero.
    // Those Ax * Bx + Ay * By = 0.

    // Bx = - Ay * By / Ax if Ax != 0, in this case, the coordinate By is taken
    // as one.
    if (!GA::equal(y(), 0)) {
        double vectorX = 1;
        double vectorY = -vectorX * x() / y();

        return {vectorX, vectorY};
    }
    // By = - Ax * Bx / Ay if Ay != 0, in this case, the coordinate Bx is taken
    // as one.
    if (!GA::equal(x(), 0)) {
        double vectorY = 1;
        double vectorX = -vectorY * y() / x();

        return {vectorX, vectorY};
    }
    // If the x and y coordinates of the current vector are zero, then return
    // the empty vector.
    return {};
}

GAVector GAVector::perpendicularNormalVector() const {
    // This function works the same way as the function, but it immediately
    // calculates the coordinates of the normalized vector.
    if (!GA::equal(y(), 0)) {
        // Calculate the coordinate of the normalized vector.
        double normalVectorX = std::sqrt(std::pow(y(), 2) / (std::pow(x(), 2) + std::pow(y(), 2)));

        double normalVectorY = -normalVectorX * x() / y();

        return {normalVectorX, normalVectorY};
    }
    if (!GA::equal(x(), 0)) {
        // Calculate the coordinate of the normalized vector.
        double normalVectorY = std::sqrt(std::pow(x(), 2) / (std::pow(x(), 2) + std::pow(y(), 2)));

        double normalVectorX = -normalVectorY * y() / x();

        return {normalVectorX, normalVectorY};
    }
    return {};
}

GAVector GAVector::reverseVector() const {
    // Simple backward direction vector calculation
    return {x() * -1, y() * -1};
}

void GAVector::normalize() {
    // To normalize a vector, divide each of its component coordinates by its
    // length.
    if (!(isNull())) {
        // Find the length of the vector to use as a divider.
        double devisor = std::sqrt(std::pow(x(), 2) + std::pow(y(), 2));

        // We divide the coordinates of the vector on the value of the divider.
        point_.setX(x() / devisor);
        point_.setY(y() / devisor);
    }
}

bool GAVector::isEmpty() const { return point_.isEmpty(); }

bool GAVector::isNull() const {
    // If both coordinates of the vector are zero, then the vector is zero.
    return GA::equal(x(), 0) && GA::equal(y(), 0);
}

double GAVector::angle(const GAVector &otherVector) const {
    // If both vectors are not null, proceed to the calculation of the angle
    // between them.
    if (!(isNull() && otherVector.isNull())) {
        // From the formula of the scalar product of vectors we derive the value
        // of the cosine of the angle between them.
        double cosinus = x() * otherVector.x() + y() * otherVector.y();

        // As a result of the error that occurs when performing calculations
        // with a double type, situations are possible in which the calculated
        // cosine value is outside the allowable range (-1; 1), for example, a
        // value of 1.00000000000000003 can be obtained, therefore, these cases
        // should be excluded.

        // If the cosine value is greater than 1 (for
        // example, 1.00000000000000003), simply return the angle 0.
        if (cosinus > 1) {
            return 0;
        }

        // If the cosine value is less than -1 (for example,
        // -1.00000000000000003), simply return the angle 180.
        if (cosinus < -1) {
            return 180;
        }

        // When returning, we translate the value from radians to degrees.
        return GA::degree(std::acos(cosinus));
    }
    // Otherwise, we assume that the angle between the vectors is zero.
    return 0;
}

double GAVector::anglePolar() const { return GA::vectorPolarAngle(point_.x(), point().y()); }

double GAVector::angleHeading() const {
    return GA::headingPolar(GA::vectorPolarAngle(point_.x(), point_.y()));
}

std::ostream &operator<<(std::ostream &os, const GAVector &vector) {
    os << "GAVector(" << vector.point_ << ")";

    return os;
}
