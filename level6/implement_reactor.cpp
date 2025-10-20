#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include "../level5/reactor.hpp"

#define BUFFER_SIZE 1024
struct Point;

// Global variables
int intflag = 0;
void *sock_reactor = nullptr;
void *stdin_reactor = nullptr;
int server_sock = -1;
std::vector<Point> points;
int graph_size = 0;

void sigint(int signum)
{
    printf("\nTerminating connection.\n");
    intflag = 1;
}

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

void processCommand(const std::string &line)
{
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    CommandType cmdType = parseCommand(command);
    try
    {
        switch (cmdType)
        {
        case CMD_NEWGRAPH:
        {
            int n;
            iss >> n;
            points.clear();
            graph_size = n;
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
            if (points.size() >= graph_size)
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
            float area = calculateArea(hull);
            std::cout << "Convex Hull Area: " << area << std::endl;
            break;
        }
        default:
            std::cout << "Command '" << command << "' could not be recognized.\n";
            break;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}
// Handler function for server socket (new connections)
void *handleServerSocket(int fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_sock < 0)
    {
        if (!intflag)
            perror("Accept failed");
        return nullptr;
    }
    printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
    // Read data from client, process line by line
    char buffer[BUFFER_SIZE];
    int bytes_received;
    std::string client_buffer;
    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        client_buffer += buffer;
        size_t pos;
        while ((pos = client_buffer.find('\n')) != std::string::npos)
        {
            std::string line = client_buffer.substr(0, pos);
            client_buffer.erase(0, pos + 1);
            // Capture processCommand output
            std::ostringstream response;
            std::streambuf* old_cout = std::cout.rdbuf(response.rdbuf());
            processCommand(line);
            std::cout.rdbuf(old_cout);
            std::string resp_str = response.str();
            if (!resp_str.empty())
                send(client_sock, resp_str.c_str(), resp_str.size(), 0);
        }
    }
    printf("Client disconnected\n");
    close(client_sock);
    return nullptr;
}

void *handleNormalFD(int fd) {
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty())
        processCommand(line);
    return nullptr;
}

int main()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint;
    sigaction(SIGINT, &act, NULL);
    int port = 9034;
    // Create server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket error");
        exit(1);
    }
    // Allow socket reuse
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(server_sock);
        exit(1);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }
    if (listen(server_sock, 10) < 0)
    {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }
    // Start reactor
    sock_reactor = reactor::startReactor();
    stdin_reactor = reactor::startReactor();
    if (!sock_reactor || !stdin_reactor)
    {
        std::cerr << "Failed to start reactor" << std::endl;
        close(server_sock);
        exit(1);
    }
    // Add server socket to reactor
    if (reactor::addFdToReactor(sock_reactor, server_sock, handleServerSocket) < 0 
        || reactor::addFdToReactor(stdin_reactor, STDIN_FILENO, handleNormalFD) < 0)
    {
        std::cerr << "Failed to add server socket to reactor" << std::endl;
        reactor::stopReactor(sock_reactor);
        reactor::stopReactor(stdin_reactor);
        close(server_sock);
        exit(1);
    }
    std::cout << "Convex Hull Server started on port " << port << std::endl;
    std::cout << "Enter commands or wait for client connections..." << std::endl;
    // Wait for interrupt signal
    while (!intflag)
    {
        sleep(1);
    }
    reactor::stopReactor(sock_reactor);
    reactor::stopReactor(stdin_reactor);
    close(server_sock);
    return 0;
}
