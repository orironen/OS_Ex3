#include "../level8/proactor.hpp"
#include <condition_variable>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/un.h>
#define BUFFER_SIZE 1024

int intflag = 0;
float ch_area = 0;
std::condition_variable cv;
std::mutex ch_mutex;

void sigint(int signum)
{
    printf("\nTerminating connection.\n");
    {
        std::lock_guard<std::mutex> lock(ch_mutex);
        intflag = 1;
    }
    cv.notify_all();
}

struct Point
{
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
    bool operator<(const Point &other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
    bool operator==(const Point &other) const {
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

// משתנים גלובליים לניהול הגרף
std::shared_ptr<std::vector<Point>> points;
int n = 0;

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
    else
        throw std::invalid_argument("Command '" + cmd + "' could not be recognized.\n");
    return CMD_UNKNOWN;
}

std::vector<Point> processCommand(CommandType cmdType, std::vector<Point> points, std::istringstream &iss)
{
    switch (cmdType)
    {
    case CMD_NEWGRAPH:
    {
        iss >> n;
        points.clear();
        std::cout << "Created new graph with size " << n << ".\n";
        break;
    }
    case CMD_NEWPOINT:
    {
        float x, y;
        char comma;
        iss >> x >> comma >> y;
        if (comma != ',')
            throw std::invalid_argument("Not comma-separated.");
        if (points.size() >= n)
            throw std::out_of_range("Graph is full.");
        points.emplace_back(x, y);
        std::cout << "Added new point (" << x << "," << y << ") to graph.\n";
        break;
    }
    case CMD_REMOVEPOINT:
    {
        float x, y;
        char comma;
        iss >> x >> comma >> y;
        if (comma != ',')
            throw std::invalid_argument("Not comma-separated.");
        Point toRemove(x, y);
        auto it = std::find(points.begin(), points.end(), toRemove);
        if (it != points.end())
        {
            points.erase(it);
            std::cout << "Removed point (" << x << "," << y << ") from graph.\n";
        }
        else
            throw std::invalid_argument("Point doesn't exist in graph.");
        break;
    }
    case CMD_CH:
    {
        std::vector<Point> hull = convexHull(points);
            {
                std::lock_guard<std::mutex> lock(ch_mutex);
                ch_area = calculateArea(hull);
            }
        cv.notify_all();
        std::cout << "Convex Hull Area: " << ch_area << std::endl;
        break;
    }
    case CMD_UNKNOWN:
        throw std::invalid_argument("Unknown command.");
    }
    return points;
}

void check_ch_area() {
    std::unique_lock<std::mutex> lock(ch_mutex);
    bool was_above = false;
    
    while(!intflag) {
        cv.wait(lock, [&](){ return intflag || (ch_area >= 100 && !was_above) || (ch_area < 100 && was_above); });
        if (intflag) return;
        if (ch_area >= 100 && !was_above) {
            std::cout << "\nAt Least 100 units belongs to CH\n";
            was_above = true;
        } else if (ch_area < 100 && was_above) {
            std::cout << "\nAt Least 100 units no longer belongs to CH\n";
            was_above = false;
        }
    }
}

void *client_handler_proactor(int client_sock)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;
    std::string line;

    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        line += buffer;
    }
    try
    {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        std::lock_guard<std::mutex> lock(designPattern::graphMutex);
        CommandType cmdType = parseCommand(command);
        auto updated_points = processCommand(cmdType, *points, iss);
        *points = std::move(updated_points);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing command: " << e.what() << std::endl;
    }

    close(client_sock);
    printf("Client disconnected\n");
    return nullptr;
}

int main()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint;
    sigaction(SIGINT, &act, NULL);
    int port = 9034;
    points = std::make_shared<std::vector<Point>>();

    std::thread t(check_ch_area);

    std::string line;
    int server_sock;
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket error");
        exit(1);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    char buffer[BUFFER_SIZE];
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed\n");
        exit(1);
    }
    if (listen(server_sock, 10) < 0)
    {
        perror("Listen failed\n");
        exit(1);
    }

    pthread_t proactor_tid = designPattern::startProactor(server_sock, client_handler_proactor);
    if (proactor_tid == 0)
    {
        std::cerr << "Failed to start proactor\n";
        close(server_sock);
        exit(1);
    }

    while (!intflag)
    {
        CommandType cmdType;
        struct pollfd pfd;
        pfd.fd = STDIN_FILENO;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, 1000) == -1)
        {
            if (intflag) continue;
            perror("Poll failed\n");
            exit(1);
        }

        line.clear();
        if (pfd.revents & POLLIN)
        {
            std::getline(std::cin, line);
            std::istringstream iss(line);
            std::string command;
            iss >> command;

            try
            {
                cmdType = parseCommand(command);
                std::lock_guard<std::mutex> lock(designPattern::graphMutex);
                *points = std::move(processCommand(cmdType, *points, iss));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }

    designPattern::stopProactor(proactor_tid);
    t.join();
    close(server_sock);
    return 0;
}
