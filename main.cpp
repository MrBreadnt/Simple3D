#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cmath>

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#define WIDTH 16 * 10
#define HEIGHT 9 * 5

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
        position = {0, 0, 0};
        rotation = {0, 0, 0};
        fov = 90.0f;
    }
};

void setPixel(int x, int y, char c, float z = 0);
void clearBuffer();
void render();
void setCursorVisible(bool visible);
Point2D screenCoords(Point2D point);
Point2D projectCoords(Point3D point, Camera& camera);
void printBorder();
void movePoint(Point3D& point, Point3D vector);
void rotateX(Point3D& point, float angle);
void rotateY(Point3D& point, float angle);
void rotateZ(Point3D& point, float angle);
Point3D computeCenter(std::vector<Point3D> vertices);
void drawLine(Point2D start, Point2D end, float z1, float z2);
void fillTriangle(Point2D v1, Point2D v2, Point2D v3, float z1, float z2, float z3, char c);
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
bool isFaceVisible(const Face& face, const Object3D& object, const Camera& camera);
void rotateCameraByPoint(Camera& camera, Point3D point, float angle, char axis);
char getShade(float z);
void fillTriangle(Point2D v1, Point2D v2, Point2D v3, float z1, float z2, float z3);
void fillObject(Object3D& object, Camera& camera);
Point3D computeFaceNormal(const Point3D& v1, const Point3D& v2, const Point3D& v3); 
float computeFaceBrightness(const Point3D& normal, const Point3D& lightDir);


HANDLE hConsole;
vector<CHAR_INFO> screenBuffer((WIDTH + 2) * (HEIGHT + 2));
COORD bufferSize = {WIDTH + 2, HEIGHT + 2};
COORD bufferCoord = {0, 0};
SMALL_RECT writeRegion = {0, 0, WIDTH + 1, HEIGHT + 1};
vector<float> zBuffer((WIDTH + 2) * (HEIGHT + 2));

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    setCursorVisible(false);

    vector<Object3D> objects;
    Camera camera;

    Object3D model;
    if (loadObjFile("model2.obj", model)) {
        scaleObject(model, 5);
        centerObject(model);
        moveObject(model, {0, 2, -10});
        model.rotationPoint = computeCenter(model.vertices);
        objects.push_back(model);
    }

    objects.push_back(generateCube({0, 0, 0,}));
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

    moveCamera(camera, {0, 0, -2});
    //rotateCamera(camera, -45, 'x');
    
    while (true) {
        clearBuffer();
        for(auto& object : objects){
            fillObject(object, camera);
        }

        rotateObject(objects[0], objects[0].rotationPoint, 5, 'y');

        //rotateCameraByPoint(camera, {0, 0, 0}, - 5 * rotateSpeed, 'y');
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

Point3D computeFaceNormal(const Point3D& v1, const Point3D& v2, const Point3D& v3) {
    float nx = (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y);
    float ny = (v2.z - v1.z) * (v3.x - v1.x) - (v2.x - v1.x) * (v3.z - v1.z);
    float nz = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
    
    return {nx, ny, nz};
}

float computeFaceBrightness(const Point3D& normal, const Point3D& lightDir) {
    float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length < 0.001) return 0.5f;
    
    float dot = (normal.x * lightDir.x + normal.y * lightDir.y + normal.z * lightDir.z) / length;
    return max(0.3f, dot);
}

void fillTriangle(Point2D v1, Point2D v2, Point2D v3, float z1, float z2, float z3, float brightness) {
    if (v1.y > v2.y) { swap(v1, v2); swap(z1, z2); }
    if (v1.y > v3.y) { swap(v1, v3); swap(z1, z3); }
    if (v2.y > v3.y) { swap(v2, v3); swap(z2, z3); }

    int y1 = (int)ceil(v1.y);
    int y2 = (int)ceil(v2.y);
    int y3 = (int)ceil(v3.y);

    int screen_y_start = max(0, y1);
    int screen_y_end = min(HEIGHT - 1, y3 - 1);

    if (screen_y_start > screen_y_end) return;

    float dy12 = v2.y - v1.y;
    float dy13 = v3.y - v1.y;
    float dy23 = v3.y - v2.y;

    float dx12 = (dy12 > 0) ? (v2.x - v1.x) / dy12 : 0;
    float dz12 = (dy12 > 0) ? (z2 - z1) / dy12 : 0;
    
    float dx13 = (dy13 > 0) ? (v3.x - v1.x) / dy13 : 0;
    float dz13 = (dy13 > 0) ? (z3 - z1) / dy13 : 0;

    float dx23 = (dy23 > 0) ? (v3.x - v2.x) / dy23 : 0;
    float dz23 = (dy23 > 0) ? (z3 - z2) / dy23 : 0;

    for (int y = screen_y_start; y <= screen_y_end; y++) {
        bool is_upper_part = y < y2;
        
        float x_a, z_a, x_b, z_b;
        
        float offset13 = (float)y - v1.y;
        x_a = v1.x + offset13 * dx13;
        z_a = z1 + offset13 * dz13;

        if (is_upper_part) {
            float offset12 = (float)y - v1.y;
            x_b = v1.x + offset12 * dx12;
            z_b = z1 + offset12 * dz12;
        } else {
            float offset23 = (float)y - v2.y;
            x_b = v2.x + offset23 * dx23;
            z_b = z2 + offset23 * dz23;
        }

        if (x_a > x_b) {
            swap(x_a, x_b);
            swap(z_a, z_b);
        }

        int x_start = max(0, (int)ceil(x_a));
        int x_end = min(WIDTH - 1, (int)ceil(x_b) - 1);

        for (int x = x_start; x <= x_end; x++) {
            float factor = (x_b != x_a) ? (x - x_a) / (x_b - x_a) : 0;
            float z = z_a + factor * (z_b - z_a);
            
            char c;
            if (brightness > 0.8) c = '@';
            else if (brightness > 0.6) c = '#';
            else if (brightness > 0.4) c = '+';
            else if (brightness > 0.2) c = ':';
            else c = '.';
            
            setPixel(x, y, c, z);
        }
    }
}

void fillObject(Object3D& object, Camera& camera) {
    Point3D lightDir = {-1, 1, 1};
    
    for (const Face& face : object.faces) {
        if (!isFaceVisible(face, object, camera)) continue;
        
        int i1 = face.vertexIndices[0] - 1;
        int i2 = face.vertexIndices[1] - 1;
        int i3 = face.vertexIndices[2] - 1;
        
        Point3D v1 = object.vertices[i1];
        Point3D v2 = object.vertices[i2];
        Point3D v3 = object.vertices[i3];
        
        Point3D normal = computeFaceNormal(v1, v2, v3);
        
        float brightness = computeFaceBrightness(normal, lightDir);
        
        for (size_t i = 1; i < face.vertexIndices.size() - 1; i++) {
            int ti1 = face.vertexIndices[0] - 1;
            int ti2 = face.vertexIndices[i] - 1;
            int ti3 = face.vertexIndices[i + 1] - 1;
            
            Point3D tv1 = object.vertices[ti1];
            Point3D tv2 = object.vertices[ti2];
            Point3D tv3 = object.vertices[ti3];
            
            Point3D cv1 = worldToCamera(tv1, camera);
            Point3D cv2 = worldToCamera(tv2, camera);
            Point3D cv3 = worldToCamera(tv3, camera);
            
            Point2D pv1 = screenCoords(projectCoords(tv1, camera));
            Point2D pv2 = screenCoords(projectCoords(tv2, camera));
            Point2D pv3 = screenCoords(projectCoords(tv3, camera));
            
            fillTriangle(pv1, pv2, pv3, cv1.z, cv2.z, cv3.z, brightness);
        }
    }
}

void rotateCameraByPoint(Camera& camera, Point3D point, float angle, char axis) {
    Point3D relativePos = {
        camera.position.x - point.x,
        camera.position.y - point.y,
        camera.position.z - point.z
    };
    
    moveCamera(camera, {point.x - camera.position.x, 
                        point.y - camera.position.y, 
                        point.z - camera.position.z});
    
    rotateCamera(camera, angle, axis);
    
    Point3D rotatedRelative = relativePos;
    rotatePoint(rotatedRelative, {0, 0, 0}, angle, axis);
    
    camera.position = {
        point.x + rotatedRelative.x,
        point.y + rotatedRelative.y,
        point.z + rotatedRelative.z
    };
}

bool isFaceVisible(const Face& face, const Object3D& object, const Camera& camera) {
    if (face.vertexIndices.size() < 3) return false;
    
    const Point3D& v1 = object.vertices[face.vertexIndices[0] - 1];
    const Point3D& v2 = object.vertices[face.vertexIndices[1] - 1];
    const Point3D& v3 = object.vertices[face.vertexIndices[2] - 1];
    
    float nx = (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y);
    float ny = (v2.z - v1.z) * (v3.x - v1.x) - (v2.x - v1.x) * (v3.z - v1.z);
    float nz = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
    
    float vx = camera.position.x - v1.x;
    float vy = camera.position.y - v1.y;
    float vz = camera.position.z - v1.z;
    
    float dot = nx * vx + ny * vy + nz * vz;
    
    return dot > 0;
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
        {{4, 3, 2, 1}},
        {{5, 6, 7, 8}},
        {{1, 2, 6, 5}},
        {{4, 8, 7, 3}},
        {{1, 5, 8, 4}},
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
        {{4, 3, 2, 1}},
        {{5, 6, 7, 8}},
        {{1, 2, 6, 5}},
        {{4, 8, 7, 3}},
        {{1, 5, 8, 4}},
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
        setPixel(x + i, y, text[i], -999999);
        i++;
    }
}

void drawObject(Object3D& object, Camera& camera){
    vector<pair<int, int>> uniqueEdges;

    for (const Face& face : object.faces) {
        if (isFaceVisible(face, object, camera))
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

        Point3D currentCamera = worldToCamera(currentPoint, camera);
        Point3D nextCamera = worldToCamera(nextPoint, camera);
        
        Point2D currentProjected = screenCoords(projectCoords(currentPoint, camera));
        Point2D nextProjected = screenCoords(projectCoords(nextPoint, camera));
        
        drawLine(currentProjected, nextProjected, currentCamera.z, nextCamera.z);
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

char getShade(float z) {
    if (z < 2) return '@';
    if (z < 4) return '%';
    if (z < 5) return '#';
    if (z < 6) return 'O';
    if (z < 7) return '*';
    if (z < 8) return '+';
    if (z < 9) return ':';
    return '.';
}

void drawLine(Point2D start, Point2D end, float z1, float z2) {
    int x1 = (int)start.x;
    int y1 = (int)start.y;
    int x2 = (int)end.x;
    int y2 = (int)end.y;
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    float totalSteps = max(abs(x2 - x1), abs(y2 - y1));
    float step = 0;
    
    while (true) {
        float t = (totalSteps > 0) ? step / totalSteps : 0;
        float currentZ = z1 + (z2 - z1) * t;
        
        setPixel(x1, y1, getShade(currentZ), currentZ);
        
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
        step++;
    }
}

void setPixel(int x, int y, char c, float z) {
    int bufferX = x + 1;
    int bufferY = y + 1;
    int index = bufferY * (WIDTH + 2) + bufferX;
    
    if (bufferX >= 0 && bufferX < WIDTH + 2 && bufferY >= 0 && bufferY < HEIGHT + 2) {
        if (z < zBuffer[index]) {
            screenBuffer[index].Char.AsciiChar = c;
            screenBuffer[index].Attributes = 7;
            zBuffer[index] = z;
        }
    }
}

void clearBuffer() {
    for (int i = 0; i < (WIDTH + 2) * (HEIGHT + 2); i++) {
        screenBuffer[i].Char.AsciiChar = ' ';
        screenBuffer[i].Attributes = 7;
        zBuffer[i] = 999999;
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
        setPixel(x - 1, -1, '-', -999999);
        setPixel(x - 1, HEIGHT, '-', -999999);
    }
    
    setPixel(-1, -1, '+', -999999);
    setPixel(WIDTH, -1, '+', -999999);
    setPixel(-1, HEIGHT, '+', -999999);
    setPixel(WIDTH, HEIGHT, '+', -999999);
    
    for (int y = 0; y < HEIGHT; y++) {
        setPixel(-1, y, '|', -999999);
        setPixel(WIDTH, y, '|', -999999);
    }
}

void setCursorVisible(bool visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}