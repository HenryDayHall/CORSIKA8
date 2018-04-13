/*
 * ParticleID.hpp
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

namespace ngc
{
    enum class ParticleID // copy of CORSIKA's particle IDs
    {
        GAMMA = 1,
        POSITRON = 2,
        ELECTRON = 3,
        MU_P = 5,
        MU_M = 6,
        PI_0 = 7,
        PI_P = 8,
        PI_M = 9,
        K_L_0 = 10,
        K_P = 11,
        K_M = 12,
        N = 13,
        P = 14,
        PBAR = 15,
        K_S_0 = 16,
        ETA = 17,
        LAMBDA = 18,
        SIGMA_P = 19,
        SIGMA_0 = 20,
        SIGMA_M = 21,
        XI_0 = 22,
        XI_M = 23,
        OMEGA_M = 24,
        N_BAR = 25,
        LAMBDA_BAR = 26,
        SIGMA_BAR_M = 27,
        SIGMA_BAR_0 = 28,
        SIGMA_BAR_P = 29,
        XI_BAR_0 = 30,
        XI_BAR_PLUS = 31,
        OMEGA_BAR_PLUS = 32
        
        // to be continued...
    };
}
