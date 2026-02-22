#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cmath>

#define WIDTH 16 * 6
#define HEIGHT 9 * 3

using namespace std;

struct Point2D {
    float x, y;
};

struct Point3D {
    float x, y, z;
};

struct Face {
    vector<int> vertexIndices;
};

struct Object3D {
    vector<Point3D> vertices;
    vector<Face> faces;
    Point3D rotationPoint;
};

struct Edge {
    int v1, v2;
    
    Edge(int a, int b) {
        v1 = min(a, b);
        v2 = max(a, b);
    }
    
    bool operator<(const Edge& other) const {
        if (v1 != other.v1) return v1 < other.v1;
        return v2 < other.v2;
    }
};

void setPixel(int x, int y, char c);
void clearBuffer();
void render();
void setCursorVisible(bool visible);
Point2D screenCoords(Point2D point);
Point2D projectCoords(Point3D point);
void printPoint(Point2D point);
void printBorder();
void rotateObject(Object3D& object, Point3D center, float angle, char axis);
void moveObject(Object3D& object, Point3D vector);
void movePoint(Point3D& point, Point3D vector);
void rotatePoint(Point3D& point, Point3D center, float angle, char axis);
void rotateX(Point3D& point, float angle);
void rotateY(Point3D& point, float angle);
void rotateZ(Point3D& point, float angle);
Point3D computeCenter(std::vector<Point3D> vertices);
void drawLine(Point2D start, Point2D end);
void drawObject(Object3D object);

HANDLE hConsole;
vector<CHAR_INFO> screenBuffer((WIDTH + 2) * (HEIGHT + 2));
COORD bufferSize = {WIDTH + 2, HEIGHT + 2};
COORD bufferCoord = {0, 0};
SMALL_RECT writeRegion = {0, 0, WIDTH + 1, HEIGHT + 1};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    setCursorVisible(false);

    vector<Object3D> objects;

    Object3D cubee;
    cubee.vertices = {
        {1, 1, 1},
        {1, -1, 1},
        {-1, -1, 1},
        {-1, 1, 1},
        {1, 1, -1},
        {1, -1, -1},
        {-1, -1, -1},
        {-1, 1, -1},
    };
    cubee.faces = {
        {{1, 2, 3, 4}},
        {{5, 6, 7, 8}},
        {{1, 2, 6, 5}},
        {{3, 4, 8, 7}},
        {{1, 4, 8, 5}},
        {{2, 3, 7, 6}}
    };
    cubee.rotationPoint = computeCenter(cubee.vertices);
    float a = 5;
    moveObject(cubee, {0, 0, 3});
    float rotationSpeed = 5;
    float moveSpeed = 0.1;
    objects.push_back(cubee);
    Object3D& cube = objects[0];
    while (true) {
        clearBuffer();

        for(Object3D object : objects){
            drawObject(object);
        }

        if (_kbhit()) {
            char key = _getch();
            
            switch (key) {
                case 'x': rotateObject(cube, cube.rotationPoint, rotationSpeed, 'x'); break;
                case 'X': rotateObject(cube, cube.rotationPoint, -rotationSpeed, 'x'); break;
                case 'y': rotateObject(cube, cube.rotationPoint, rotationSpeed, 'y'); break;
                case 'Y': rotateObject(cube, cube.rotationPoint, -rotationSpeed, 'y'); break;
                case 'z': rotateObject(cube, cube.rotationPoint, rotationSpeed, 'z'); break;
                case 'Z': rotateObject(cube, cube.rotationPoint, -rotationSpeed, 'z'); break;
                
                case 'w': moveObject(cube, {0, moveSpeed, 0}); break;
                case 's': moveObject(cube, {0, -moveSpeed, 0}); break;
                case 'a': moveObject(cube, {-moveSpeed, 0, 0}); break;
                case 'd': moveObject(cube, {moveSpeed, 0, 0}); break;
                case 'q': moveObject(cube, {0, 0, moveSpeed}); break;
                case 'e': moveObject(cube, {0, 0, -moveSpeed}); break;
                
                case 27:
                    setCursorVisible(true);
                    return 0;
            }
        }

        printBorder();
        
        render();
        
        Sleep(50);
    }

    setCursorVisible(true);
    return 0;
}

void drawObject(Object3D object){
    vector<pair<int, int>> uniqueEdges;

    for (const Face& face : object.faces) {
        for (size_t i = 0; i < face.vertexIndices.size(); i++) {
            int v1 = face.vertexIndices[i] - 1;
            int v2 = face.vertexIndices[(i + 1) % face.vertexIndices.size()] - 1;
            
            if (v1 > v2) swap(v1, v2);
            bool alreadyExists = false;
            for (size_t j = 0; j < uniqueEdges.size(); j++) {
                if (uniqueEdges[j].first == v1 && uniqueEdges[j].second == v2) {
                    alreadyExists = true;
                    break;
                }
            }
            
            if (!alreadyExists) {
                uniqueEdges.push_back({v1, v2});
            }
        }
    }

    for (size_t j = 0; j < uniqueEdges.size(); j++) {
        Point3D currentPoint = object.vertices[uniqueEdges[j].first];
        Point3D nextPoint = object.vertices[uniqueEdges[j].second];
        
        Point2D currentProjected = screenCoords(projectCoords(currentPoint));
        Point2D nextProjected = screenCoords(projectCoords(nextPoint));
        
        drawLine(currentProjected, nextProjected);
    }
}

void rotateObject(Object3D& object, Point3D center, float angle, char axis){
    for (Point3D& point : object.vertices){
        rotatePoint(point, center, angle, axis);
    }
}

void moveObject(Object3D& object, Point3D vector){
    for (Point3D& point : object.vertices){
        movePoint(point, vector);
    }
    movePoint(object.rotationPoint, vector);
}

void movePoint(Point3D& point, Point3D vector){
    point.x += vector.x;
    point.y += vector.y;
    point.z += vector.z;
}

void rotatePoint(Point3D& point, Point3D center, float angle, char axis) {
    float x = point.x - center.x;
    float y = point.y - center.y;
    float z = point.z - center.z;
    
    Point3D relative = {x, y, z};
    
    if (axis == 'x') rotateX(relative, angle);
    else if (axis == 'y') rotateY(relative, angle);
    else if (axis == 'z') rotateZ(relative, angle);
    
    point.x = relative.x + center.x;
    point.y = relative.y + center.y;
    point.z = relative.z + center.z;
}

void rotateX(Point3D& point, float angle) {
    float rad = angle * 3.14159 / 180;
    float cosA = cos(rad);
    float sinA = sin(rad);
    
    float newY = point.y * cosA - point.z * sinA;
    float newZ = point.y * sinA + point.z * cosA;
    
    point.y = newY;
    point.z = newZ;
}

void rotateY(Point3D& point, float angle) {
    float rad = angle * 3.14159 / 180;
    float cosA = cos(rad);
    float sinA = sin(rad);
    
    float newX = point.x * cosA + point.z * sinA;
    float newZ = -point.x * sinA + point.z * cosA;
    
    
    point.x = newX;
    point.z = newZ;
}

void rotateZ(Point3D& point, float angle) {
    float rad = angle * 3.14159 / 180;
    float cosA = cos(rad);
    float sinA = sin(rad);
    
    float newX = point.x * cosA - point.y * sinA;
    float newY = point.x * sinA + point.y * cosA;
    
    point.y = newY;
    point.x = newX;
}

Point3D computeCenter(vector<Point3D> vertices) {
    Point3D center = {0, 0, 0};
    
    if (vertices.empty()) return center;
    
    for (const auto vertex : vertices) {
        center.x += vertex.x;
        center.y += vertex.y;
        center.z += vertex.z;
    }
    
    center.x /= vertices.size();
    center.y /= vertices.size();
    center.z /= vertices.size();
    
    return center;
}

void drawLine(Point2D start, Point2D end) {
    int x1 = (int)start.x;
    int y1 = (int)start.y;
    int x2 = (int)end.x;
    int y2 = (int)end.y;
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        setPixel(x1, y1, '*');
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void setPixel(int x, int y, char c) {
    int bufferX = x + 1;
    int bufferY = y + 1;
    
    if (bufferX >= 0 && bufferX < WIDTH + 2 && bufferY >= 0 && bufferY < HEIGHT + 2) {
        screenBuffer[bufferY * (WIDTH + 2) + bufferX].Char.AsciiChar = c;
        screenBuffer[bufferY * (WIDTH + 2) + bufferX].Attributes = 7;
    }
}

void clearBuffer() {
    for (int i = 0; i < (WIDTH + 2) * (HEIGHT + 2); i++) {
        screenBuffer[i].Char.AsciiChar = ' ';
        screenBuffer[i].Attributes = 7;
    }
}

void render() {
    WriteConsoleOutput(hConsole, screenBuffer.data(), bufferSize, bufferCoord, &writeRegion);
}

Point2D screenCoords(Point2D point) {
    point.x = (point.x + 1) * (WIDTH - 1) / 2;
    point.y = (-point.y + 1) * (HEIGHT - 1) / 2;
    return point;
}

Point2D projectCoords(Point3D point){
    if (point.z == 0) return {-2, -2};
    point.x /= point.z;
    point.y /= point.z;
    return {point.x, point.y};
}

void printPoint(Point2D point) {
    setPixel((int)point.x, (int)point.y, '*');
}

void printBorder() {
    for (int x = 0; x < WIDTH + 2; x++) {
        setPixel(x - 1, -1, '-');
        setPixel(x - 1, HEIGHT, '-');
    }
    
    setPixel(-1, -1, '+');
    setPixel(WIDTH, -1, '+');
    setPixel(-1, HEIGHT, '+');
    setPixel(WIDTH, HEIGHT, '+');
    
    for (int y = 0; y < HEIGHT; y++) {
        setPixel(-1, y, '|');
        setPixel(WIDTH, y, '|');
    }
}

void setCursorVisible(bool visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}