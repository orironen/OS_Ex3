#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <string>

struct Point
{
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
    bool operator<(const Point &other) const
    {
        return x < other.x || (x == other.x && y < other.y);
    }
};

float crossProduct(const Point &O, const Point &A, const Point &B)
{
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// מימוש עם vector
std::vector<Point> convexHullVector(const std::vector<Point> &points)
{
    int size = points.size();
    if (size <= 1)
        return points;
    std::vector<Point> sorted_points = points; // יצירת עותק
    std::sort(sorted_points.begin(), sorted_points.end());
    std::vector<Point> lower, upper;
    for (int i = 0; i < size; i++)
    {
        while (lower.size() >= 2 &&
               crossProduct(lower[lower.size() - 2], lower[lower.size() - 1], sorted_points[i]) <= 0)
            lower.pop_back();
        lower.push_back(sorted_points[i]);
    }
    for (int i = size - 1; i >= 0; i--)
    {
        while (upper.size() >= 2 && crossProduct(upper[upper.size() - 2], upper[upper.size() - 1], sorted_points[i]) <= 0)
            upper.pop_back();
        upper.push_back(sorted_points[i]);
    }

    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

// מימוש עם deque
std::deque<Point> convexHullDeque(const std::vector<Point> &points)
{
    int size = points.size();
    if (size <= 1)
        return std::deque<Point>(points.begin(), points.end());

    std::vector<Point> sorted_points = points; // יצירת עותק
    std::sort(sorted_points.begin(), sorted_points.end());

    std::deque<Point> lower, upper;
    for (int i = 0; i < size; i++)
    {
        while (lower.size() >= 2 &&
               crossProduct(*(lower.rbegin() + 1), *(lower.rbegin()), sorted_points[i]) <= 0)
            lower.pop_back();
        lower.push_back(sorted_points[i]);
    }

    for (int i = size - 1; i >= 0; i--)
    {
        while (upper.size() >= 2 &&
               crossProduct(*(upper.rbegin() + 1), *(upper.rbegin()), sorted_points[i]) <= 0)
            upper.pop_back();
        upper.push_back(sorted_points[i]);
    }

    lower.pop_back();
    upper.pop_back();
    for (const auto &p : upper)
        lower.push_back(p);
    return lower;
}

template <typename Container>
float calculateArea(const Container &hull)
{
    if (hull.size() < 3)
        return 0;
    float area = 0;
    int size = hull.size();
    for (int i = 0; i < size; i++)
    {
        const Point &p1 = hull[i];
        const Point &p2 = hull[(i + 1) % size];
        area += p1.x * p2.y - p2.x * p1.y;
    }

    return std::abs(area) / 2.0;
}

int main()
{
    int numPoints;
    std::cout << "Enter num of points:\n";
    std::cin >> numPoints;
    std::vector<Point> points(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        char comma;
        std::cout << "Enter point " << i << "(comma-separated): ";
        std::cin >> points[i].x >> comma >> points[i].y;
    }

    std::string mode;
    std::cout << "Enter mode (vector/deque): ";
    std::cin >> mode;
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    float area = 0;
    if (mode == "vector")
    {
        std::vector<Point> hull = convexHullVector(points);
        area = calculateArea(hull);
    }
    else if (mode == "deque")
    {
        std::deque<Point> hull = convexHullDeque(points);
        area = calculateArea(hull);
    }
    else
    {
        std::cerr << "Invalid mode\n";
        return 1;
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    std::cout << "Convex Hull Area: " << area << std::endl;
    std::cerr << "Time: " << duration.count() << " microseconds" << std::endl;
    return 0;
}
