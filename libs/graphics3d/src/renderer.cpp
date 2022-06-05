//
// Renderer
//
#include "graphics3d/base.h"
#include "graphics3d/renderer.h"

#include <cmath>
#include <cstring>

using namespace graphics;

static const float PI = 3.14159274101257324219f;
static const float PI_12 = PI*0.5f;

// ############################################################################
// Init
// ############################################################################

Renderer::Renderer() {
}

Renderer::~Renderer() {
    free();
}

void Renderer::init(graphics::Display* display, bool enable_zbuffer) {
    display_ = display;

    // calculate screen ratio
    display_ratio_ = (float) display_->width() / (float) display_->height();

    if (enable_zbuffer) {
        alloc();
    }

    camera_.set(0.0f, 0.0f, -4.0f);
    light_.set(0.0f, 0.0f, -10.0f);
}

void Renderer::update() {
    if (nullptr != screen_buffer_) {
        flushBuffers();
    }
}

// ############################################################################
// Z-Buffer
// ############################################################################

void Renderer::alloc() {
    free();
    screen_buffer_ = new graphics::Bitmap(display_->width(), display_->height(), 16);
}

void Renderer::free() {
    if (nullptr != screen_buffer_) {
        delete screen_buffer_;
        screen_buffer_ = nullptr;
    }
}

void Renderer::flushBuffers() {
    if (nullptr == screen_buffer_) return;

    auto width = screen_buffer_->width();
    auto height = screen_buffer_->height();
    auto bytesPerLine = screen_buffer_->bytesPerLine();
    auto bitsPerPixel = screen_buffer_->bitsPerPixel();
    auto src_buffer = (const uint16_t*) screen_buffer_->pixels();
    auto dest_buffer = display_->device()->buffer();

    for (auto y = 0; y<height; y++) {
        auto mask = 1 << (y & 7);
        auto src = (y * bytesPerLine * 8) / bitsPerPixel;
        for (auto x = 0; x<width; x++) {

            uint16_t z_buffer_pixel = src_buffer[src];
            int col = decodeColor(z_buffer_pixel);

            auto dest = x + (y / 8) * width;
            if (0 != col)
                dest_buffer[dest] |= mask;
            else
                dest_buffer[dest] &= ~mask;

            src ++;
        }
    }

    { // clear z-buffer
        auto buffer = screen_buffer_->lock();
        std::memset(buffer, 0x0, screen_buffer_->size());
        screen_buffer_->unlock();
    }

}

inline uint16_t Renderer::encode(int col, float z) {
    uint16_t pixel = encodeZ(z);
    if (col != 0) pixel |= 0x8000;
    return pixel;
}

inline int Renderer::decodeColor(uint16_t pixel) {
    return (0 != (pixel & 0x8000)) ? 1 : 0;
}

inline int Renderer::maskZ(uint16_t pixel) {
    return ((int) pixel & 0x7fff);
}

inline float Renderer::decodeZ(uint16_t pixel) {
    int z_range = maskZ(pixel) - 0x4000;
    float z = - (float) z_range / 1024.0f;
    return z;
}

inline uint16_t Renderer::encodeZ(float z) {
    int z_encoded = (int) (-z * 1024.0f) + 0x4000;
    if (z_encoded < 0) z_encoded = 0;
    if (z_encoded > 0x7fff) z_encoded = 0x7fff;
    return (uint16_t) z_encoded;
}

// ############################################################################
// 3D Projection
// ############################################################################

void Renderer::project(const Mesh* mesh) {

    const auto& rotation = mesh->rotation();
    const auto& position = mesh->position();
    const auto& scale = mesh->scale();
    const auto& vertices = mesh->vertices();

    // ensure cache buffer size is enough
    if (projection_cache_.size() < vertices.size()) {
        projection_cache_.resize(vertices.size());
    }

    // buffer sin/cos for rotation
    float s_x = std::sin(rotation.x);
    float c_x = std::cos(rotation.x);
    float s_y = std::sin(rotation.y);
    float c_y = std::cos(rotation.y);

    // project vertices to screen coordinates

    size_t index = 0;

    for (auto& vertex : vertices) {

        Point p = vertex.coords;
        p *= scale;

        auto& p2 = projection_cache_[index];

        p2.set(
            (p.x * c_y + p.z * s_y),
            p.y * c_x - (p.z * c_y - p.x * s_y) * s_x,
            (p.z * c_y - p.x * s_y) * c_x + p.y * s_x
        );

        p2 += position;

        index++;
    }

}

Point2 Renderer::toScreen(const Point& p) {
    Point2 s;

    float z_dist = p.z - camera_.z;

    float z_factor = z_dist >= 0.00001f ? 1.0f / z_dist : 100000.0f;

    auto x_norm = (p.x - camera_.x) * z_factor;
    auto y_norm = (p.y - camera_.y) * z_factor;

    float w = (float) display_->width() * 0.5f;
    float h = (float) display_->height() * 0.5f;

    s.x = (int)  (w + (x_norm * w));
    s.y = (int)  (h + (y_norm * h * display_ratio_));

    return s;
}

// ############################################################################
// High-Level Drawing
// ############################################################################

void Renderer::drawMesh(const Mesh* mesh, bool draw_wireframe) {

    const auto& faces = mesh->faces();
    const auto& vertices = projection_cache_;
    const auto& light = light_;

    if (faces.empty()) return;

    project(mesh);

    Point v3;
    Point2 s3;
    Vector center;

    for (const auto& face : faces) {

        int num_vertices = face.size;

        // render quadric
        // fetch current quadric
        const auto& v0 = vertices[face.a];
        auto s0 = toScreen(v0);

        const auto& v1 = vertices[face.b];
        auto s1 = toScreen(v1);

        const auto& v2 = vertices[face.c];
        auto s2 = toScreen(v2);

        if (4 == num_vertices) {
            v3 = vertices[face.d];
            s3 = toScreen(v3);

            // calculate center of quadric
            center = {
                (v0.x+v1.x+v2.x+v3.x)/4.0f,
                (v0.y+v1.y+v2.y+v3.y)/4.0f,
                (v0.z+v1.z+v2.z+v3.z)/4.0f
            };
        } else {
            // calculate center of triangle
            center = {
                (v0.x+v1.x+v2.x)/3.0f,
                (v0.y+v1.y+v2.y)/3.0f,
                (v0.z+v1.z+v2.z)/3.0f
            };
        }

        // do some maths to calculate surface normal
        Vector normal = Vector::crossProduct((v1 - v0), (v1 - v2)).normalize();

        // get surface orientation against light source
        Vector line = Vector::subtract(center, light).normalize();
        float angle = std::asin((line.x * normal.x + line.y * normal.y + line.z * normal.z));

        // do not draw if surface is facing backwards
        if (backface_culling_ && angle <= 0.0f) continue;

        // calculate intensity from angle
        int col = (int) (angle / PI_12 * 256.0f);
        if (col < 0) col = 0;
        if (col > 255) col = 255;

        if (4 == num_vertices) {

            // render quadric with two triangles
            drawTriangle(s0, v0.z, s1, v1.z, s3, v3.z, col);
            drawTriangle(s1, v1.z, s2, v2.z, s3, v3.z, col);

            // render wire-frame overlay
            if (draw_wireframe) {
                drawLine(s0, v0.z, s1, v1.z);
                drawLine(s1, v1.z, s2, v2.z);
                drawLine(s2, v2.z, s3, v3.z);
                drawLine(s0, v0.z, s1, v1.z);
            }

        } else if (3 == num_vertices) {

            // render quadric with two triangles
            drawTriangle(s0, v0.z, s1, v1.z, s2, v2.z, col);

            // render wire-frame overlay
            if (draw_wireframe) {
                drawLine(s0, v0.z, s1, v1.z);
                drawLine(s1, v1.z, s2, v2.z);
                drawLine(s0, v0.z, s2, v2.z);
            }
        }
    }
}

// ############################################################################
// Low-Level Pixel Drawing
// ############################################################################

void Renderer::drawPixelClipped(int x, int y, float z, int col) {
    if ((x >= display_->width()) || (x < 0) || (y >= display_->height()) || (y < 0)) return;

    auto buffer = (uint16_t*) screen_buffer_->lock();
    auto index = (y * screen_buffer_->bytesPerLine() * 8) / screen_buffer_->bitsPerPixel() + x;

    uint16_t z_old = maskZ(buffer[index]);
    uint16_t z_new = encodeZ(z);

    if (z_new >= z_old) {
        buffer[index] = z_new | ((col != 0) ? 0x8000 : 0x0);
    }

    screen_buffer_->unlock();
}

// ############################################################################
// Low-Level Triangle Drawing
// ############################################################################

void Renderer::drawTriangle(const Point2& a, float az, const Point2& b, float bz, const Point2& c, float cz, int intensity) {
    if (screen_buffer_) {
        fillDitheredTriangle(a.x, a.y, az, b.x, b.y, bz, c.x, c.y, cz, intensity);
    } else {
        display_->fillDitheredTriangle(a.x, a.y, b.x, b.y, c.x, c.y, intensity);
    }
}

void Renderer::fillDitheredTriangle(int x1, int y1, float z1, int x2, int y2, float z2, int x3, int y3, float z3, int intensity) {

    // Note: No clipping is done. This could lead to very long worst case execution times!

    int h = screen_buffer_->height();

    // sort
    if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }
    if (y1 > y3) { std::swap(x1, x3); std::swap(y1, y3); }
    if (y2 > y3) { std::swap(x2, x3); std::swap(y2, y3); }

    if (y3 < 0) return;

    int total_height = (y3 - y1) + 1;

    int last_min = -1;
    float last_min_z = 0.0f;
    int last_max = 128;
    float last_max_z = 0.0f;

    int ofs = (y1 >= 0) ? 0 : -y1;

    for (int i = ofs; i < total_height; i++) {

        bool second_half = (i > y2 - y1) || (y2 == y1);
        int segment_height = second_half ? y3 - y2 : y2 - y1;
        if (segment_height < 1) continue;

        float alpha = (float) i / (float) total_height;
        float beta  = (float) (i - (second_half ? (y2-y1) : 0)) / (float) segment_height;

        int ax = x1 + (x3 - x1) * alpha;
        float az = z1 + (z3 - z1) * alpha;

        int bx = second_half ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;
        float bz = second_half ? z2 + (z3 - z2) * beta : z1 + (z2 - z1) * beta;

        if (ax > bx) {
            std::swap(ax, bx);
            std::swap(az, bz);
        }

        // avoid gaps
        if (ax > last_max) { ax = last_max; az = last_max_z; }
        if (bx < last_min) { bx = last_min; bz = last_min_z; }

        // draw
        drawDitheredHorizontalLine(ax, y1 + i, az, bx, bz, intensity);

        if (y1 + i >= h) break;

        last_min = ax; last_max = bx;
        last_min_z = az; last_max_z = bz;
    }
}

void Renderer::fillDitheredRectangle(int x1, int y1, int x2, int y2, float z, int intensity) {

    sort_pair(x1, x2);
    sort_pair(y1, y2);

    for (auto i = y1; i <= y2; ++i) {
        drawDitheredHorizontalLine(x1, i, z, x2, z, intensity);
    }
}

// ############################################################################
// Low-Level Line Drawing
// ############################################################################

void Renderer::drawDitheredHorizontalLine(int x, int y, float z1, int x2, float z2, int intensity) {
    auto width = screen_buffer_->width();
    auto height = screen_buffer_->height();
    auto bytesPerLine = screen_buffer_->bytesPerLine();
    auto bitsPerPixel = screen_buffer_->bitsPerPixel();

    if (x2 < x) { // unsure left to right direction
        int tmp = x; x = x2; x2 = tmp;
        tmp = z1; z1 = z2; z2 = tmp;
    }

    uint8_t w = x2 - x + 1;

    // check boundaries

    if ((x >= width) || (x + w - 1 < 0) || (y >= height) || (y < 0)) {
        return;
    }

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (x + w > width) {
        w = width - x;
    }

    if (w == 0) {
        return;
    }

    auto buffer = (uint16_t*) screen_buffer_->lock();
    auto index = (y * bytesPerLine * 8) / bitsPerPixel + x;

    float z_diff = z2 - z1;

    for (auto i=0; i<w; i++) {
        float z = z1 + (z_diff * (float) i) / (float) w;

        uint16_t z_old = maskZ(buffer[index]);
        uint16_t z_new = encodeZ(z);

        if (x < 0 || x >= width) {
            break; //
        }

        if (z_new >= z_old) {
            auto col = display_->getDitheredColor(x, y, intensity) ? graphics::Color::WHITE : graphics::Color::BLACK;
            buffer[index] = z_new | ((col != 0) ? 0x8000 : 0x0);
        }

        index++;
        ++x;
    }

    screen_buffer_->unlock();
}

void Renderer::drawLine(const Point2& a, float az, const Point2& b, float bz) {
    if (screen_buffer_) {
        drawLine(a.x, a.y, az, b.x, b.y, bz, 1);
    } else {
        display_->drawLine(a.x, a.y, b.x, b.y);
    }
}

void Renderer::drawLine(int x0, int y0, float z0, int x1, int y1, float z1, int col) {

    int w = screen_buffer_->width();
    int h = screen_buffer_->height();
    int bitsPerPixel = screen_buffer_->bitsPerPixel();
    int bytesPerLine = screen_buffer_->bytesPerLine();

    if (x0 < 0 && x1 < 0) return;
    if (x0 >= w && x1 >= w) return;
    if (y0 < 0 && y1 < 0) return;
    if (y0 >= h && y1 >= h) return;

    float dx = (float) (x1 - x0);
    float dy = (float) (y1 - y0);
    float dz = (z1 - z0);

    if (std::abs(dx) >= std::abs(dy)) {

        if (x1 < x0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            std::swap(z0, z1);
        }

        if (x1 < 0 || x0 >= w) return;

        dy = (dx != 0.0f) ? dy / dx : 0.0f;
        dz = (dx != 0.0f) ? dz / dx : 0.0f;

        float y = (float) y0;
        float z = z0;

        if (x0 < 0) {
            y -= ((float) x0 * dy);
            z -= ((float) x0 * dz);
            x0 = 0;
        }

        if (x1 > w-1) {
            x1 = w-1;
        }

        auto buffer = (uint16_t*) screen_buffer_->lock();

        for (auto x=x0; x<=x1; x++) {
            int yi = (int) y;
            if (yi >= 0 && yi < h) {
                auto index = (yi * bytesPerLine * 8) / bitsPerPixel + x;
                if (lines_ignore_zbuffer_) {
                    buffer[index] = 0x7fff | ((col != 0) ? 0x8000 : 0x0);
                } else {
                    uint16_t z_old = maskZ(buffer[index]);
                    uint16_t z_new = encodeZ(z);
                    if (z_new >= z_old) {
                        buffer[index] = z_new | ((col != 0) ? 0x8000 : 0x0);
                    }
                }
            }
            y += dy; z += dz;
        }

        screen_buffer_->unlock();

    } else {

        if (y1 < y0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            std::swap(z0, z1);
        }

        if (y1 < 0 || y0 >= h) return;

        dx = (dy != 0.0f) ? dx / dy : 0.0f;
        dz = (dy != 0.0f) ? dz / dy : 0.0f;

        float x = (float) x0;
        float z = z0;

        if (y0 < 0) {
            x -= ((float) y0 * dx);
            z -= ((float) y0 * dz);
            y0 = 0;
        }

        if (y1 > h-1) {
            y1 = h-1;
        }

        auto buffer = (uint16_t*) screen_buffer_->lock();

        for (auto y=y0; y<=y1; y++) {
            int xi = (int) x;
            if (xi >= 0 && xi < w) {
                auto index = (y * bytesPerLine * 8) / bitsPerPixel + xi;

                if (lines_ignore_zbuffer_) {
                    buffer[index] = 0x7fff | ((col != 0) ? 0x8000 : 0x0);
                } else {
                    uint16_t z_old = maskZ(buffer[index]);
                    uint16_t z_new = encodeZ(z);
                    if (z_new >= z_old) {
                        buffer[index] = z_new | ((col != 0) ? 0x8000 : 0x0);
                    }
                }
            }
            x += dx; z += dz;
        }

        screen_buffer_->unlock();
    }
}
