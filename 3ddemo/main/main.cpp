//
// Graphics Demo
//

#include "application/application.h"
#include "graphics/graphics.h"
#include "graphics3d/graphics3d.h"
#include <vector>
#include <cmath>

#include "mesh.inc"

class Graphics3D : public application::Application {
   public:
    explicit Graphics3D() : Application()  {
    }

    void init() override {
        setPeriod(40);                                          // set 40 ms cycle / 25 fps
        showStatistics(false);                                  // disable statistics overlay (default)

        createMesh();                                           // create 3D mesh

        auto display = getDisplay();                            // get display reference

        display->setBuiltinFont(1);                             // set font
        display->clear();                                       // clear display
        display->update(true);                                  // update display

        renderer_.init(display);                                // initialize renderer
    }

    void update() override {
        auto display = getDisplay();                            // get display reference
        display->clear();                                       // clear display (ignore locked areas)

        auto delta = this->getDelta();
        mesh_.rotate({1.23f*delta, 2.47f*delta, 0.0f});         // update rotation angles
        renderer_.drawMesh(&mesh_, false);                       // draw 3D mesh
        renderer_.update();                                     // update render buffers

        display->update();                                      // update screen
    }

    void createMesh() {
        mesh_.setVertices(vertices);
        mesh_.setFaces(faces);
        mesh_.setPosition({ 0.0f, 0.0f, 0.0f });
        mesh_.setScale({ 1.0f, 1.0f, 1.0f });
        mesh_.setRotation({ 0.0f, 0.0f, 0.0f });
    }

    private:
        graphics::Renderer renderer_;
        graphics::Mesh mesh_;

    _NODEFAULTS(Graphics3D)
};

DECLARE_APP(Graphics3D);
