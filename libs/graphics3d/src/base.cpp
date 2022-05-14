//
// Renderer
//
#include "graphics3d/base.h"
#include "graphics3d/renderer.h"

#include <cmath>

using namespace graphics;

static const float PI = 3.14159274101257324219f;
static const float PI_2 = PI*2.0f;

////////////////////////////////////////////////////////////////////////////////
// Vector
////////////////////////////////////////////////////////////////////////////////

Vector::Vector() : x{0.0f}, y{0.0f}, z{0.0f} {}

Vector::Vector(float _x, float _y, float _z) : x{_x}, y{_y}, z{_z} {}

void Vector::set(float _x, float _y, float _z) {
    x = _x; y = _y; z = _z;
}

Vector Vector::crossProduct(const Vector& a, const Vector& b) {
    Vector p {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
    return p;
}

float Vector::dotProduct(const Vector& a, const Vector& b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

Vector Vector::normalize() const {
    if (x == 0.0f && y == 0.0f && z == 0.0f) return Vector(0.0f, 0.0f, 0.0f);
    float len = std::sqrt(x*x + y*y + z*z);

    Vector p {
        x / len,
        y / len,
        z / len
    };

    return p;
}

Vector Vector::subtract(const Vector& a, const Vector&b) {
    return Vector {
        a.x-b.x,
        a.y-b.y,
        a.z-b.z
    };
}

Vector Vector::add(const Vector& a, const Vector&b) {
    return Vector {
        a.x+b.x,
        a.y+b.y,
        a.z+b.z
    };
}

float Vector::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

Vector& Vector::operator=(const Vector& p) {
    x = p.x;
    y = p.y;
    z = p.z;
    return *this;
}

Vector& Vector::operator-=(const Vector& p) {
    x -= p.x;
    y -= p.y;
    z -= p.z;
    return *this;
}

Vector& Vector::operator+=(const Vector& p) {
    x += p.x;
    y += p.y;
    z += p.z;
    return *this;
}

Vector& Vector::operator*=(const Vector& p) {
    x *= p.x;
    y *= p.y;
    z *= p.z;
    return *this;
}

Vector operator-(const Vector& a, const Vector& b) {
    return Vector(a.x-b.x, a.y-b.y, a.z-b.z);
}

Vector operator+(const Vector& a, const Vector& b) {
    return Vector(a.x+b.x, a.y+b.y, a.z+b.z);
}

////////////////////////////////////////////////////////////////////////////////
// Face
////////////////////////////////////////////////////////////////////////////////

Face::Face(int _a, int _b, int _c) : a(_a), b(_b), c(_c), d(0), size(3) {}

Face::Face(int _a, int _b, int _c, int _d) : a(_a), b(_b), c(_c), d(_d), size(4) {}

////////////////////////////////////////////////////////////////////////////////
// Vertex
////////////////////////////////////////////////////////////////////////////////

Vertex::Vertex() {}

Vertex::Vertex(float _x, float _y, float _z) : coords{_x, _y,_z} {}

void Vertex::set(float _x, float _y, float _z) {
    coords.set(_x, _y, _z);
}

////////////////////////////////////////////////////////////////////////////////
// Mesh
////////////////////////////////////////////////////////////////////////////////

void Mesh::setVertices(const std::vector<Vertex>& vertices) {
    clearVertices();
    vertices_ref_ = &vertices;
}

void Mesh::setVertices(std::vector<Vertex>&& vertices) {
    clearVertices();
    vertices_ = vertices;
}

void Mesh::clearVertices() {
    vertices_ref_ = nullptr;
    vertices_.clear();
}

void Mesh::setFaces(const std::vector<Face>& faces) {
    clearFaces();
    faces_ref_ = &faces;
}

void Mesh::setFaces(const std::vector<Face>&& faces) {
    clearFaces();
    faces_ = faces;
}

void Mesh::clearFaces() {
    faces_ref_ = nullptr;
    faces_.clear();
}

void Mesh::setPosition(const Point& position) {
    position_ = position;
}

void Mesh::setPosition(float x, float y, float z) {
    position_.set(x, y, z);
}

void Mesh::setScale(const Vector& scale) {
    scale_ = scale;
}

void Mesh::setScale(float x, float y, float z) {
    scale_.set(x, y, z);
}

void Mesh::setRotation(const Vector& rotation) {
    rotation_ = rotation;
}

void Mesh::setRotation(float x, float y, float z) {
    rotation_.set(x, y, z);
}

void Mesh::rotate(const Vector& rotation) {
    rotation_.x += rotation.x;
    if (rotation_.x > PI_2) rotation_.x -= PI_2;
    if (rotation_.x < 0.0f) rotation_.x += PI_2;

    rotation_.y += rotation.y;
    if (rotation_.y > PI_2) rotation_.y -= PI_2;
    if (rotation_.y < 0.0f) rotation_.y += PI_2;

    rotation_.z += rotation.z;
    if (rotation_.z > PI_2) rotation_.z -= PI_2;
    if (rotation_.z < 0.0f) rotation_.z += PI_2;
}

const Point& Mesh::position() const {
    return position_;
}

Point& Mesh::position() {
    return position_;
}

const Vector& Mesh::scale() const {
    return scale_;
}

Vector& Mesh::scale() {
    return scale_;
}

const Vector& Mesh::rotation() const {
    return rotation_;
}

Vector& Mesh::rotation() {
    return rotation_;
}

const std::vector<Vertex>& Mesh::vertices() const {
    if (nullptr != vertices_ref_) return *vertices_ref_;
    return vertices_;
}

const std::vector<Face>& Mesh::faces() const {
    if (nullptr != faces_ref_) return *faces_ref_;
    return faces_;
}
