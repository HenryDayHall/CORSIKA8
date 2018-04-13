/*
 * python.cpp
 * 
 * Copyright 2018 Maximilian Reininghaus <maximilian.reininghaus@kit.edu>
 * 
 * This file is part of ngC.
 * 
 * ngC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ngC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <pybind11/pybind11.h>
#include "ParticleID.hpp"

/* 
 * to compile, cd to ngc/build and
 *     g++ -shared -fPIC -std=c++14 -I /usr/include/python3.5/ \
 *     -I ../pybind11/include/  -I ../include \
 *     -o example`python3-config --extension-suffix` ../src/python.cpp
 * then open a python3 shell and import example
 */

PYBIND11_MODULE(example, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
        
    pybind11::enum_<ngc::ParticleID>(m, "ParticleID")
        .value("gamma", ngc::ParticleID::GAMMA)
        .value("positron", ngc::ParticleID::POSITRON)
        .value("electron", ngc::ParticleID::ELECTRON)
        .value("mu_p", ngc::ParticleID::MU_P)
        .value("mu_m", ngc::ParticleID::MU_M);
        // to be continued...
}
