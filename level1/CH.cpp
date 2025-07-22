#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

struct Point
{
    float x, y;
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
};

// פונקציה לחישוב מכפלה וקטורית-קובעת את כיוון הסיבוב
float crossProduct(const Point &O, const Point &A, const Point &B)
{
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> convexHull(std::vector<Point> points)
{
    int size = points.size();
    if (size <= 1)
        return points;
    sort(points.begin(), points.end());
    std::vector<Point> hull;
    // חלק תחתון
    for (int i = 0; i < size; i++)
    {
        while (hull.size() >= 2 && crossProduct(hull[hull.size() - 2], hull[hull.size() - 1], points[i]) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }
    // חלק עליון
    int t = hull.size() + 1;
    for (int i = size - 2; i >= 0; i--)
    {
        while (hull.size() >= t && crossProduct(hull[hull.size() - 2], hull[hull.size() - 1], points[i]) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }
    hull.pop_back(); // הסרת נקודה הכפולה
}
