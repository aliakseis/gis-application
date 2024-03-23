#include "gapoint.h"

#include "gavector.h"
#include "gautils.h"

GAPoint::GAPoint() : isEmpty_(true), x_(0), y_(0) {}

GAPoint::GAPoint(double x, double y, bool isEmpty) : isEmpty_(isEmpty), x_(x), y_(y) {}

GAPoint::GAPoint(const GAPoint& point) {
    x_ = point.x_;
    y_ = point.y_;
    isEmpty_ = point.isEmpty_;
}

GAPoint& GAPoint::operator=(const GAPoint& otherPoint) = default;

bool GAPoint::operator==(const GAPoint& otherPoint) const {
    return GA::equal(x_, otherPoint.x_) && GA::equal(y_, otherPoint.y_);
}

bool GAPoint::operator!=(const GAPoint& otherPoint) const { return !(*this == otherPoint); }

double GAPoint::x() const { return x_; }

void GAPoint::setX(double x) {
    x_ = x;
    isEmpty_ = false;
}

double GAPoint::y() const { return y_; }

void GAPoint::setY(double y) {
    y_ = y;
    isEmpty_ = false;
}

bool GAPoint::isEmpty() const { return isEmpty_; }

void GAPoint::setIsEmpty(bool isEmpty) { isEmpty_ = isEmpty; }

void GAPoint::setOffset(double offsetX, double offsetY) {
    x_ += offsetX;
    y_ += offsetY;
    isEmpty_ = false;
}

double GAPoint::distance(const GAPoint& otherPoint) const {
    return std::hypot(x_ - otherPoint.x_, y_ - otherPoint.y_);
}

std::ostream& operator<<(std::ostream& os, const GAPoint& point) {
    std::streamsize precisionStream = os.precision();
    std::ios_base::fmtflags formatFlagsStream = os.flags();

    os.precision(10);
    os.setf(std::ios_base::fixed);

    os << "GAPoint(" << point.x_ << ", " << point.y_ << ")";

    os.precision(precisionStream);
    os.flags(formatFlagsStream);

    return os;
}
