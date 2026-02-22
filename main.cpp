#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cmath>

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#define WIDTH 16 * 20
#define HEIGHT 9 * 10

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

struct Camera {
    Point3D position;
    Point3D rotation;
    float fov;
    
    Camera() {
        position = {0, 0, -5};
        rotation = {0, 0, 0};
        fov = 90.0f;
    }
};

void setPixel(int x, int y, char c);
void clearBuffer();
void render();
void setCursorVisible(bool visible);
Point2D screenCoords(Point2D point);
Point2D projectCoords(Point3D point, Camera& camera);
void printPoint(Point2D point);
void printBorder();
void movePoint(Point3D& point, Point3D vector);
void rotateX(Point3D& point, float angle);
void rotateY(Point3D& point, float angle);
void rotateZ(Point3D& point, float angle);
Point3D computeCenter(std::vector<Point3D> vertices);
void drawLine(Point2D start, Point2D end);
void drawObject(Object3D& object, Camera& camera);
Point3D worldToCamera(Point3D worldPoint, Camera& camera);
void rotateCamera(Camera& camera, float angle, char axis);
void moveCamera(Camera& camera, Point3D vector);
void drawText(int x, int y, const char* text);
void drawCameraInfo(Camera& camera);
bool loadObjFile(const char* filename, Object3D& object);
void centerObject(Object3D& object);
void scaleObject(Object3D& object, float scale);
void rotatePoint(Point3D& point, Point3D center, float angle, char axis);
void moveObject(Object3D& object, Point3D vector);
void rotateObject(Object3D& object, Point3D center, float angle, char axis);
Object3D generateCube(Point3D position);
Object3D generateCube(Point3D position, Point3D rotation, float scale);

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
    Camera camera;
    Object3D model;
    if (loadObjFile("model.obj", model)) {
        scaleObject(model, 5);
        centerObject(model);
        moveObject(model, {0, 2, 2});
        //objects.push_back(model);
    }

    objects.push_back(generateCube({0, 0, 3}));
    objects.push_back(generateCube({1, 0, 3}));
    objects.push_back(generateCube({-1, 0, 3}));
    objects.push_back(generateCube({0, 1, 3}));
    objects.push_back(generateCube({0, 2, 3}));
    objects.push_back(generateCube({0, 3, 3}));
    objects.push_back(generateCube({1, 3, 3}));
    objects.push_back(generateCube({-1, 3, 3}));
    
    float moveSpeed = 0.2;
    float rotateSpeed = 2;
    
    while (true) {
        clearBuffer();

        for(auto& object : objects){
            drawObject(object, camera);
        }

        if (GetAsyncKeyState('I') & 0x8000) rotateCamera(camera, -rotateSpeed, 'x');
        if (GetAsyncKeyState('K') & 0x8000) rotateCamera(camera, rotateSpeed, 'x');
        
        if (GetAsyncKeyState('Y') & 0x8000) rotateCamera(camera, -rotateSpeed, 'y');
        if (GetAsyncKeyState('U') & 0x8000) rotateCamera(camera, rotateSpeed, 'y');
        
        if (GetAsyncKeyState('W') & 0x8000) moveCamera(camera, {0, 0, moveSpeed});
        if (GetAsyncKeyState('S') & 0x8000) moveCamera(camera, {0, 0, -moveSpeed});
        if (GetAsyncKeyState('A') & 0x8000) moveCamera(camera, {-moveSpeed, 0, 0});
        if (GetAsyncKeyState('D') & 0x8000) moveCamera(camera, {moveSpeed, 0, 0});
        if (GetAsyncKeyState('Q') & 0x8000) moveCamera(camera, {0, -moveSpeed, 0});
        if (GetAsyncKeyState('E') & 0x8000) moveCamera(camera, {0, moveSpeed, 0});
        
        if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000 || GetAsyncKeyState(VK_ADD) & 0x8000) 
            camera.fov = min(150.0f, camera.fov + 5);
        
        if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000 || GetAsyncKeyState(VK_SUBTRACT) & 0x8000) 
            camera.fov = max(30.0f, camera.fov - 5);
        
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            setCursorVisible(true);
            return 0;
        }

        printBorder();
        drawCameraInfo(camera);
        render();
        Sleep(50);
    }

    setCursorVisible(true);
    return 0;
}

Object3D generateCube(Point3D position){
    Object3D cube;
    cube.vertices = {
        {0.5, 0.5, 0.5},
        {0.5, -0.5, 0.5},
        {-0.5, -0.5, 0.5},
        {-0.5, 0.5, 0.5},
        {0.5, 0.5, -0.5},
        {0.5, -0.5, -0.5},
        {-0.5, -0.5, -0.5},
        {-0.5, 0.5, -0.5},
    };
    cube.faces = {
        {{1, 2, 3, 4}},
        {{5, 6, 7, 8}},
        {{1, 2, 6, 5}},
        {{3, 4, 8, 7}},
        {{1, 4, 8, 5}},
        {{2, 3, 7, 6}}
    };
    cube.rotationPoint = computeCenter(cube.vertices);
    moveObject(cube, position);
    return cube;
}

Object3D generateCube(Point3D position, Point3D rotation, float scale){
    Object3D cube;
    cube.vertices = {
        {0.5, 0.5, 0.5},
        {0.5, -0.5, 0.5},
        {-0.5, -0.5, 0.5},
        {-0.5, 0.5, 0.5},
        {0.5, 0.5, -0.5},
        {0.5, -0.5, -0.5},
        {-0.5, -0.5, -0.5},
        {-0.5, 0.5, -0.5},
    };
    cube.faces = {
        {{1, 2, 3, 4}},
        {{5, 6, 7, 8}},
        {{1, 2, 6, 5}},
        {{3, 4, 8, 7}},
        {{1, 4, 8, 5}},
        {{2, 3, 7, 6}}
    };
    cube.rotationPoint = computeCenter(cube.vertices);
    rotateObject(cube, cube.rotationPoint, rotation.x, 'x');
    rotateObject(cube, cube.rotationPoint, rotation.y, 'y');
    rotateObject(cube, cube.rotationPoint, rotation.z, 'z');
    scaleObject(cube, scale);
    moveObject(cube, position);
    return cube;
}

void scaleObject(Object3D& object, float scale) {
    for (auto& vertex : object.vertices) {
        vertex.x *= scale;
        vertex.y *= scale;
        vertex.z *= scale;
    }
    object.rotationPoint = computeCenter(object.vertices);
}

void centerObject(Object3D& object) {
    Point3D center = computeCenter(object.vertices);
    for (auto& vertex : object.vertices) {
        vertex.x -= center.x;
        vertex.y -= center.y;
        vertex.z -= center.z;
    }
    object.rotationPoint = {0, 0, 0};
}

bool loadObjFile(const char* filename, Object3D& object) {
    object.vertices.clear();
    object.faces.clear();
    
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    string line;
    int lineNum = 0;
    
    while (getline(file, line)) {
        lineNum++;
        
        if (line.empty()) continue;
        
        istringstream iss(line);
        string type;
        iss >> type;
        
        if (type == "v") {
            Point3D vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            object.vertices.push_back(vertex);
        }
        
        else if (type == "f") {
            Face face;
            string vertexData;
            
            while (iss >> vertexData) {
                replace(vertexData.begin(), vertexData.end(), '/', ' ');
                istringstream viss(vertexData);
                
                int vertexIndex;
                viss >> vertexIndex;
                
                if (vertexIndex < 0) {
                    vertexIndex = object.vertices.size() + vertexIndex + 1;
                }
                
                face.vertexIndices.push_back(vertexIndex);
                
                int dummy;
                viss >> dummy;
                viss >> dummy;
            }
            
            if (face.vertexIndices.size() >= 3) {
                object.faces.push_back(face);
            }
        }
    }
    
    file.close();
    
    if (!object.vertices.empty()) {
        object.rotationPoint = computeCenter(object.vertices);
    }
    
    return true;
}


void drawCameraInfo(Camera& camera) {
    char buffer[100];
    sprintf(buffer, "Pos: X=%.1f Y=%.1f Z=%.1f", 
            camera.position.x, camera.position.y, camera.position.z);
    drawText(0, 0, buffer);
    
    sprintf(buffer, "Rot: X=%.1f Y=%.1f Z=%.1f", 
            camera.rotation.x, camera.rotation.y, camera.rotation.z);
    drawText(0, 1, buffer);
    
    sprintf(buffer, "FOV: %.0f", camera.fov);
    drawText(0, 2, buffer);
}

Point3D worldToCamera(Point3D worldPoint, Camera& camera) {
    Point3D relative = {
        worldPoint.x - camera.position.x,
        worldPoint.y - camera.position.y,
        worldPoint.z - camera.position.z
    };
    
    float yaw = camera.rotation.y * 3.14159 / 180;
    float cosY = cos(yaw);
    float sinY = sin(yaw);
    
    Point3D afterYaw;
    afterYaw.x = relative.x * cosY - relative.z * sinY;
    afterYaw.y = relative.y;
    afterYaw.z = relative.x * sinY + relative.z * cosY;
    
    float pitch = camera.rotation.x * 3.14159 / 180;
    float cosP = cos(pitch);
    float sinP = sin(pitch);
    
    Point3D afterPitch;
    afterPitch.x = afterYaw.x;
    afterPitch.y = afterYaw.y * cosP - afterYaw.z * sinP;
    afterPitch.z = afterYaw.y * sinP + afterYaw.z * cosP;
    
    float roll = camera.rotation.z * 3.14159 / 180;
    float cosR = cos(roll);
    float sinR = sin(roll);
    
    Point3D result;
    result.x = afterPitch.x * cosR - afterPitch.y * sinR;
    result.y = afterPitch.x * sinR + afterPitch.y * cosR;
    result.z = afterPitch.z;
    
    return result;
}

void rotateCamera(Camera& camera, float angle, char axis) {
    switch (axis) {
        case 'x':
            camera.rotation.x += angle;
            break;
        case 'y':
            camera.rotation.y += angle;
            break;
    }
    
    if (camera.rotation.x > 360) camera.rotation.x -= 360;
    if (camera.rotation.x < 0) camera.rotation.x += 360;
    if (camera.rotation.y > 360) camera.rotation.y -= 360;
    if (camera.rotation.y < 0) camera.rotation.y += 360;
    if (camera.rotation.z > 360) camera.rotation.z -= 360;
    if (camera.rotation.z < 0) camera.rotation.z += 360;
}

void moveCamera(Camera& camera, Point3D vector) {
    float yaw = camera.rotation.y * 3.14159 / 180;
    float pitch = camera.rotation.x * 3.14159 / 180;
    
    float cosY = cos(yaw);
    float sinY = sin(yaw);
    float cosP = cos(pitch);
    float sinP = sin(pitch);
    
    Point3D worldVector;
    
    if (vector.z != 0) {
        worldVector.x = vector.z * sinY * cosP;
        worldVector.y = 0;
        worldVector.z = vector.z * cosY * cosP;
    }
    
    if (vector.x != 0) {
        worldVector.x += vector.x * cosY;
        worldVector.y += 0;
        worldVector.z += -vector.x * sinY;
    }
    
    if (vector.y != 0) {
        worldVector.x += 0;
        worldVector.y += vector.y;
        worldVector.z += 0;
    }
    
    camera.position.x += worldVector.x;
    camera.position.y += worldVector.y;
    camera.position.z += worldVector.z;
}
void drawText(int x, int y, const char* text) {
    int i = 0;
    while (text[i] != '\0') {
        setPixel(x + i, y, text[i]);
        i++;
    }
}

void drawObject(Object3D& object, Camera& camera){
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
        
        Point2D currentProjected = screenCoords(projectCoords(currentPoint, camera));
        Point2D nextProjected = screenCoords(projectCoords(nextPoint, camera));
        
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


void movePoint(Point3D& point, Point3D vector) {
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
    
    point.x = newX;
    point.y = newY;
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

Point2D projectCoords(Point3D point, Camera& camera) {
    Point3D cameraPoint = worldToCamera(point, camera);
    
    float fovRad = camera.fov * 3.14159 / 180;
    float scale = 1.0f / tan(fovRad / 2);
    
    if (cameraPoint.z <= 0) return {-2, -2};
    
    float projectedX = (cameraPoint.x * scale) / cameraPoint.z;
    float projectedY = (cameraPoint.y * scale) / cameraPoint.z;
    
    return {projectedX, projectedY};
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