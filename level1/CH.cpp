#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

struct Point
{
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
    bool operator<(const Point &other) const
    {
        return x < other.x || (x == other.x && y < other.y);
    }
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
    std::vector<Point> lower;
    for (int i = 0; i < size; i++)
    {
        while (lower.size() >= 2 && crossProduct(lower[lower.size() - 2], lower[lower.size() - 1], points[i]) <= 0)
        {
            lower.pop_back();
        }
        lower.push_back(points[i]);
    }
    // חלק עליון
    std::vector<Point> upper;
    for (int i = size - 1; i >= 0; i--)
    {
        while (upper.size() >= 2 && crossProduct(upper[upper.size() - 2], upper[upper.size() - 1], points[i]) <= 0)
        {
            upper.pop_back();
        }
        upper.push_back(points[i]);
    }
    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

float calculateArea(const std::vector<Point> &hull)
{
    if (hull.size() < 3)
        return 0;
    float area = 0;
    int size = hull.size();
    for (int i = 0; i < size; i++)
    {
        int j = (i + 1) % size;
        area += hull[i].x * hull[j].y;
        area -= hull[j].x * hull[i].y;
    }

    return std::abs(area) / 2.0;
}

int main()
{
    int numPoints;
    std::cin >> numPoints;
    std::vector<Point> points(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        char comma;
        std::cin >> points[i].x >> comma >> points[i].y;
    }
    std::vector<Point> hull = convexHull(points);
    float area = calculateArea(hull);
    std::cout << area << std::endl;
    return 0;
}