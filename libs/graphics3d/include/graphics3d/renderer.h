//
// 3D Graphics
//

#pragma once

#include "graphics/graphics.h"
#include "graphics3d/base.h"
#include "graphics3d/renderer.h"
#include <vector>

namespace graphics {

/**
 * Renderer
 */
class Renderer {

    public:
        Renderer();
        ~Renderer();

    public:
        void init(graphics::Display* display, bool enable_zbuffer=true);
        void update();

    public:
        void drawMesh(const Mesh* mesh, bool draw_wireframe=false);

    private:
        void alloc();
        void free();
        void flushBuffers();
        void project(const Mesh* mesh);
        Point2 toScreen(const Point& p);

    public: // private:
        void drawPixelClipped(int x, int y, float z, int col);
        void drawLine(const Point2& a, float az, const Point2& b, float bz);
        void drawLine(int x0, int y0, float z0, int x1, int y1, float z1, int col);
        void drawDitheredHorizontalLine(int x, int y, float z1, int x2, float z2, int intensity);
        void drawTriangle(const Point2& a, float az, const Point2& b, float bz, const Point2& c, float cz, int intensity);
        void fillDitheredTriangle(int x1, int y1, float z1, int x2, int y2, float z2, int x3, int y3, float z3, int intensity);
        void fillDitheredRectangle(int x1, int y1, int x2, int y2, float z, int intensity);

    private:
        static inline uint16_t encode(int col, float z);
        static inline int decodeColor(uint16_t pixel);
        static inline uint16_t encodeZ(float z);
        static inline float decodeZ(uint16_t pixel);
        static inline int maskZ(uint16_t pixel);

    private:
        graphics::Display* display_;
        graphics::Bitmap* screen_buffer_{nullptr};
        std::vector<Point> projection_cache_;
        float display_ratio_{1.0f};
        Point camera_;
        Point light_;
        bool backface_culling_{true};
        bool lines_ignore_zbuffer_{false};
};

}  // namespace
