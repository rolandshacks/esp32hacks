//
// Base types and definitions
//
#pragma once

#include <cstdint>
#include <cstddef>

#include <vector>

namespace graphics {

/**
 * Vector
 */
class Vector {
    public:
        float x;
        float y;
        float z;

    public:
        Vector();
        Vector(float _x, float _y, float _z);

    public:
        void set(float _x, float _y, float _z);

    public:
        static Vector crossProduct(const Vector& a, const Vector& b);
        static float dotProduct(const Vector& a, const Vector& b);
        Vector normalize() const;
        static Vector subtract(const Vector& a, const Vector&b);
        static Vector add(const Vector& a, const Vector&b);
        float length() const;

    public:
        Vector& operator=(const Vector& p);
        Vector& operator-=(const Vector& p);
        Vector& operator+=(const Vector& p);
        Vector& operator*=(const Vector& p);
};

typedef Vector Point; ///< Point type. Alias for vector for better semantics.

} // namespace

graphics::Vector operator-(const graphics::Vector& a, const graphics::Vector& b);
graphics::Vector operator+(const graphics::Vector& a, const graphics::Vector& b);

namespace graphics {

/**
 * Face
 */
class Face {
    public:
        int a;
        int b;
        int c;
        int d;

    public:
        size_t size{0};

    public:
        Face(int _a, int _b, int _c);
        Face(int _a, int _b, int _c, int _d);
};


/**
 * Vertex data
 */
class Vertex {
    public:
        Point coords;

    public:
        Vertex();
        Vertex(float _x, float _y, float _z);

    public:
        void set(float _x, float _y, float _z);
};

/**
 * Mesh data
 */
class Mesh {

    public:
        void setVertices(const std::vector<Vertex>& vertices);
        void setVertices(std::vector<Vertex>&& vertices);
        void clearVertices();

    public:
        void setFaces(const std::vector<Face>& faces);
        void setFaces(const std::vector<Face>&& faces);
        void clearFaces();

    public:
        void setPosition(const Point& position);
        void setPosition(float x, float y, float z);

        void setScale(const Vector& scale);
        void setScale(float x, float y, float z);

        void setRotation(const Vector& rotation);
        void setRotation(float x, float y, float z);
        void rotate(const Vector& rotation);

    public: // getters
        const Point& position() const;
        Point& position();

        const Vector& scale() const;
        Vector& scale();

        const Vector& rotation() const;
        Vector& rotation();

        const std::vector<Vertex>& vertices() const;
        const std::vector<Face>& faces() const;

    private:
        // vertices and faces
        std::vector<Vertex> vertices_;
        const std::vector<Vertex>* vertices_ref_{nullptr};

        std::vector<Face> faces_;
        const std::vector<Face>* faces_ref_{nullptr};

    private:
        // mesh parameters
        // note: simplification, usually not directly stored at a mesh
        Point position_ {0.0f, 0.0f, 0.0f};
        Vector scale_ {1.0f, 1.0f, 1.0f};
        Vector rotation_ {0.0f, 0.0f, 0.0f};
};

}  // namespace
