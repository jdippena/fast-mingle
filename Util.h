#ifndef MINGLEC_POINT_H
#define MINGLEC_POINT_H

#include <cmath>
#include <vector>
#include <cassert>
#include <functional>

static const double PHI = (1 + sqrt(5)) / 2;
static const int UNGROUPED = -1;


#define POINT_ID_NONE    0


typedef uint32_t PointId;

static PointId numPoints = 0;

struct Point {
    float x = 0, y = 0;
    PointId id;

    Point(double x2, double y2) : x((float) x2), y((float) y2), id(++numPoints) {}
    Point(float x2, float y2) : x(x2), y(y2), id(++numPoints) {}
    Point() : x(0.0f), y(0.0f), id(POINT_ID_NONE) {}
    Point operator+ (Point& p);
    Point operator- (Point& p);
    Point operator* (int k);
    Point operator/ (double k);
    bool operator==(const Point& other) const;
    double operator* (Point& p);
    double inline norm() { return sqrt(x * x + y * y); }
    double inline sqDist(const Point& p) {
        double dx = x - p.x, dy = y - p.y;
        return dx * dx + dy * dy;
    };
};

struct PointHasher {
    std::size_t operator()(const Point& p) const;
};


Point lerp(const Point &a, const Point &b, const double delta);
double costFunction(std::vector<Point>& S, std::vector<Point>& T, Point& sPoint, Point& tPoint);


#endif //MINGLEC_POINT_H
