#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>

struct Point
{
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
    bool operator<(const Point &other) const
    {
        return x < other.x || (x == other.x && y < other.y);
    }
    bool operator==(const Point &other) const
    {
        return x == other.x && y == other.y;
    }
};

enum CommandType
{
    CMD_NEWGRAPH,
    CMD_NEWPOINT,
    CMD_REMOVEPOINT,
    CMD_CH,
    CMD_UNKNOWN
};

float crossProduct(const Point &O, const Point &A, const Point &B)
{
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> convexHull(std::vector<Point> points)
{
    int n = points.size();
    if (n <= 1)
        return points;
    std::sort(points.begin(), points.end());
    std::vector<Point> lower, upper;
    for (int i = 0; i < n; ++i)
    {
        while (lower.size() >= 2 && crossProduct(lower[lower.size() - 2], lower[lower.size() - 1], points[i]) <= 0)
            lower.pop_back();
        lower.push_back(points[i]);
    }
    for (int i = n - 1; i >= 0; --i)
    {
        while (upper.size() >= 2 && crossProduct(upper[upper.size() - 2], upper[upper.size() - 1], points[i]) <= 0)
            upper.pop_back();
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
    int n = hull.size();
    for (int i = 0; i < n; ++i)
    {
        int j = (i + 1) % n;
        area += hull[i].x * hull[j].y;
        area -= hull[j].x * hull[i].y;
    }
    return std::abs(area) / 2.0;
}

CommandType parseCommand(const std::string &cmd)
{
    if (cmd == "Newgraph")
        return CMD_NEWGRAPH;
    if (cmd == "Newpoint")
        return CMD_NEWPOINT;
    if (cmd == "Removepoint")
        return CMD_REMOVEPOINT;
    if (cmd == "CH")
        return CMD_CH;
    return CMD_UNKNOWN;
}

int main()
{
    std::vector<Point> points;
    std::string line;
    while (std::getline(std::cin, line))
    {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        CommandType cmdType = parseCommand(command);
        switch (cmdType)
        {
        case CMD_NEWGRAPH:
        {
            int n;
            iss >> n;
            points.clear();
            for (int i = 0; i < n; ++i)
            {
                std::getline(std::cin, line);
                float x, y;
                char comma;
                std::istringstream pointLine(line);
                pointLine >> x >> comma >> y;
                points.emplace_back(x, y);
            }
            break;
        }
        case CMD_NEWPOINT:
        {
            float x, y;
            char comma;
            iss >> x >> comma >> y;
            points.emplace_back(x, y);
            break;
        }
        case CMD_REMOVEPOINT:
        {
            float x, y;
            char comma;
            iss >> x >> comma >> y;
            Point toRemove(x, y);
            auto it = std::find(points.begin(), points.end(), toRemove);
            if (it != points.end())
            {
                points.erase(it);
            }
            break;
        }
        case CMD_CH:
        {
            std::vector<Point> hull = convexHull(points);
            float area = calculateArea(hull);
            std::cout << area << std::endl;
            break;
        }
        default:
            // Ignore unknown command
            break;
        }
    }
    return 0;
}
