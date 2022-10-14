#include "gdexample.h"

#include "Basis.hpp"
#include "GodotGlobal.hpp"
#include "Math.hpp"

//#include <Viewport.hpp>
//#include <ViewportTexture.hpp>
//#include <Image.hpp>
#include <MultiMeshInstance.hpp>
#include <MultiMesh.hpp>

#include <cstdlib>

#include <algorithm> // std::min

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "util.hpp"
#include <numeric>
#include <limits>
#include "rng.hpp"

using namespace godot;

// https://en.cppreference.com/w/cpp/thread/call_once
std::once_flag flagInitPython;
void initPython_do_once()
{
    std::call_once(flagInitPython, [](){
        Godot::print("Initializing Python...");
        py::initialize_interpreter();
        std::atexit([](){
            Godot::print("Deinitializing Python...");
            py::finalize_interpreter();
            Godot::print("Deinitialized Python"); /*TODO: fflush(stdout) of some sort should be here after this print() ideally*/
        });
        Godot::print("Initialized Python"); /*TODO: fflush(stdout) of some sort should be here after this print() ideally*/
        
//         // Hack to fix PYTHONPATH
//         py::exec(R"(from __future__ import print_function
// import sys
// print('sys.path:',sys.path)
// import os
// p="/nix/store/41prvfp9n0x56xvi5s4xagshwqj8cyj9-python-2.7.18-env/lib/python2.7/site-packages"
// assert(os.path.exists(p)) # If this assertion fails, check what is present in Nix `python2 -c 'import sys; print(sys.path)'` but missing in the line starting with `sys.path:` that was printed above.
// sys.path.append(p)
// sys.path.append(os.path.realpath('..')))"
//                  , py::globals());
    });
}

void GDExample::_register_methods() {
    register_method("_process", &GDExample::_process);
}

GDExample::GDExample()
    : rng(RandomNumberGenerator::_new()), air_rng(RandomNumberGenerator::_new()),
      sigma(125.7),
      epsilon(0.3345)
{
    initPython_do_once();

    int64_t seed = 1024;
    rng->set_seed(seed);
    air_rng->set_seed(seed);
    
    // Make molecules
    size_t numAir = 100;
    size_t num = 100 + numAir;
    real_t epsilon = 10; //1;
    real_t max = 7.5 * epsilon; //7.5;
    real_t velmax = 20;
    molecules.reserve(num);
    for (size_t i = 0; i < num; i++) {
        molecules.push_back({
                sim::MoleculeType::Argon,
                Vector3(rng->randf_range(-max, max), rng->randf_range(-max, max), rng->randf_range(-max, max)),
                //Vector3::ZERO
                Vector3(rng->randf_range(-velmax, velmax), rng->randf_range(-velmax, velmax), rng->randf_range(-velmax, velmax))
            });
    }

    // Make air
    std::vector<real_t> airComposition = {0.7808, 0.2095, 0.0093, 0.0004}; // https://en.wikipedia.org/wiki/Atmosphere_of_Earth : "By mole fraction (i.e., by number of molecules), dry air contains 78.08% nitrogen, 20.95% oxygen, 0.93% argon, 0.04% carbon dioxide, and small amounts of other gases."
    for (size_t i = 0; i < numAir; i++) {
        sim::MoleculeType m;
        switch (rng::randp(*air_rng, airComposition)) {
        case 0:
            m = sim::Nitrogen;
            break;
        case 1:
            m = sim::Oxygen;
            break;
        case 2:
            m = sim::Argon;
            break;
        case 3:
            m = sim::CarbonDioxide;
            break;
        default:
            exit(1);
        }
    }

    // Make walls
    num = 1000;
    max = max * (1.0 + 1.0 / 3); //10;
    walls.reserve(num);
    for (real_t i = -max; i < max; i+=epsilon) {
        for (real_t j = -max; j < max; j+=epsilon) {
            for (real_t k = -max; k < max; k+=epsilon) {
                if ((i < max - epsilon && j < max - epsilon && k < max - epsilon) && (i > -max && j > -max && k > -max)) continue;
                walls.push_back({
                        Vector3(i,j,k)
                    });
            }
        }
    }
    
    static_assert(sizeof(uint64_t) >= sizeof(size_t));
    Godot::print("Walls: {0}", (uint64_t)walls.size()); // Based on `String("Hello, {0}!").format(Array::make(target))` in code example on https://gamedevadventures.posthaven.com/using-c-plus-plus-and-gdnative-in-godot-part-1
}

GDExample::~GDExample() {
    // add your cleanup here
    rng->free();
    air_rng->free();
}

void GDExample::initSim(double timeSkip_) {
    // initialize any variables here
    currentRowIndex = 0;
    timePassedTotal += timePassed;
    timePassed = 0.0;
    timeSkip = 0.0;
    running = true;
    updateNumber = 0;
    
    // Configurable //
    timeScale = 0.80; //1;//15; //90; //5; //0.5; //0.1;
    posScale = 1; //0.00001; //0.001;
    // //
    
    std::string s = "timeSkip: " + std::to_string(timeSkip);
    Godot::print(s.c_str());
    s = "currentRowIndex: " + std::to_string(currentRowIndex);
    Godot::print(s.c_str());
}

void GDExample::_init() {
    originalTransform = get_transform();
    timePassed = 0.0;
    initSim();
}

double GDExample::currentTime() const {
    return timePassed + timeSkip;
}

// void GDExample::_input(Variant event) {
//     Ref<InputEventKey> btn = event;
//     if (btn->is_pressed() && btn->get_scancode() == GlobalConstants::KEY_R) {
//         timeScale += 0.05;
//     }
// }

void GDExample::_process(float delta) {
    if (!running) return;
    // if (timePassedTotal + timePassed > flightDuration) {
    //     std::string s = "Simulation finished";
    //     Godot::print(s.c_str());
    //     running = false;
    //     return;
    // }
    
    size_t numFrames = SIZE_MAX;//(py::int_)trajectory.attr("__len__")();
    if (updateNumber >= numFrames) {
        Godot::print("Simulation finished");
        running = false;
        return;
    }

    // Grab MultiMeshInstance
    // Based on https://www.youtube.com/watch?v=XPcSfXsoArQ
    MultiMeshInstance* mmi = (MultiMeshInstance*)get_node("Molecules");
    Ref<MultiMesh> mm = mmi->get_multimesh();

    // Loop over all atoms' positions in the coords list
    size_t numAtoms = molecules.size();
    mm->set_instance_count(numAtoms + walls.size());
    for (size_t i = 0; i < numAtoms; i++) {
        sim::Molecule& m = molecules[i];
        Vector3 pos = m.pos;
        mm->set_instance_transform(i, Transform(Basis(), pos));
    }

    // Loop over walls
    //Godot::print("Color format: {0}", mm->get_color_format());
    for (size_t i = 0; i < walls.size(); i++) {
        sim::Wall& m = walls[i];
        Vector3 pos = m.pos;
        mm->set_instance_transform(numAtoms + i, Transform(Basis(), pos));
        mm->set_instance_color(numAtoms + i, Color(0.3, 0.4, 0.5));
    }

    // Grab bonds //
    
    // // Grab MultiMeshInstance
    // mmi = (MultiMeshInstance*)get_node("Bonds");
    // mm = mmi->get_multimesh();

    // py::object bonds = molecule.attr("connectivity").attr("bonds");
    // size_t numBonds = (py::int_)bonds.attr("__len__")();
    // assert((ssize_t)numBonds >= 0); // Assert ">= 0"
    // mm->set_instance_count(numBonds);
    // locals["bonds"] = bonds;
    // py::object bond;
    // std::pair<size_t, size_t> bondAtomIndices;
    // for (size_t i = 0; i < numBonds; i++) {
    //     locals["i"] = i;
    //     bond = py::eval(R"(bonds[i])", py::globals(), locals);
    //     bondAtomIndices = {(size_t)(py::int_)bond.attr("i"), (size_t)(py::int_)bond.attr("j")};
    //     assert((ssize_t)bondAtomIndices.first >= 0 && (ssize_t)bondAtomIndices.second >= 0); // Assert ">= 0"
    //     assert(bondAtomIndices.first < numAtoms && bondAtomIndices.second < numAtoms); // Assert in bounds of the atom coords list

    //     // Grab the atom coords for this bond's ends
    //     locals["i"] = bondAtomIndices.first;
    //     locals["j"] = bondAtomIndices.second;
    //     py::tuple atomCoords__ = (py::tuple)py::eval(R"((x[i], x[j]))", py::globals(), locals);
    //     std::pair<py::object, py::object> atomCoords_ = {atomCoords__[0], atomCoords__[1]};
        
    //     locals["vec1"] = atomCoords_.first;
    //     locals["vec2"] = atomCoords_.second;
    //     std::pair<Vector3, Vector3> atomCoords = {
    //         Vector3((real_t)(py::float_)py::eval("vec1[0]", locals), (real_t)(py::float_)py::eval("vec1[1]", locals), (real_t)(py::float_)py::eval("vec1[2]", locals)), Vector3((real_t)(py::float_)py::eval("vec2[0]", locals), (real_t)(py::float_)py::eval("vec2[1]", locals), (real_t)(py::float_)py::eval("vec2[2]", locals))
    //     };

    //     Vector3 atomsVec = atomCoords.second - atomCoords.first; // The vector between the atoms
    //     auto x = atomsVec.x, y = atomsVec.y, z = atomsVec.z;
    //     // https://community.khronos.org/t/converting-a-3d-vector-into-three-euler-angles/49889/3
    //     // "Let r = radius, t = angle on x-y plane, & p = angle off of z-axis."
    //     auto r = sqrt(x*x + y*y + z*z); // radius
    //     auto t = atan(y/x); // yaw (our convention here is chosen for this to be: about z axis)
    //     auto p = acos(z/r); // pitch (convention chosen: about x axis)
    //     real_t roll = 0; // roll (convention chosen: about y axis)
    //     Vector3 bondDirection = atomsVec.normalized();
        
    //     auto transform = Transform().looking_at(bondDirection, Vector3::UP); // Following tip on https://godotengine.org/qa/77346/moving-and-rotating-trees-in-multimesh : "first rotate then reposition"
    //     transform.rotate(transform.basis.x, M_PI/2);
    //     transform.origin = util::midpoint(atomCoords.first, atomCoords.second);
    //     mm->set_instance_transform(i, transform);
        
    //     mm->set_instance_custom_data(i, Color(r, 0, 0, 0)); // Says Color() but really isn't -- it becomes a per-instance `INSTANCE_CUSTOM` variable within the shader. This is a silly function in Godot, should be improved on their side to support variable amounts of data instead of only 4 or nothing.
    // } 
    // // //


    // Simulate a bit
    sim::iterate(sigma, epsilon, delta/10, molecules, walls);
    

    updateNumber++;
}
