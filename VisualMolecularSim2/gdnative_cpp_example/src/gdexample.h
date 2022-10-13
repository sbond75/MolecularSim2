#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <Godot.hpp>
#include <Sprite3D.hpp>
#include <RigidBody.hpp>

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "sim.hpp"
#include <RandomNumberGenerator.hpp>

namespace godot {

// class GDExample : public Sprite3D {
//     GODOT_CLASS(GDExample, Sprite3D)
class GDExample : public Spatial {
    GODOT_CLASS(GDExample, Spatial)

private:
    size_t currentRowIndex;
    double timePassed, timePassedTotal; // The accumulated delta time.
    double timeSkip; // Time to skip in the file.
    Vector3 velocity; // Current running total of accumulated velocity (after running integration of acceleration).
    double timeScale; // Speeds up time
    double posScale; // Scales all positions to a smaller overall area
    bool running;
    size_t updateNumber;
    Vector3 adjustmentForPosition;

    Transform originalTransform;

    std::vector<sim::Argon> molecules;
    RandomNumberGenerator* rng;
    real_t sigma;
    real_t epsilon;
    
    double currentTime() const;
    
public:
    static void _register_methods();

    GDExample();
    ~GDExample();

    void _init(); // our initializer called by Godot

    void initSim(double timeSkip_=0.0);

    void _process(float delta);

    void _draw();
};

}

#endif
