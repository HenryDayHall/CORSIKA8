/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/qgsjetII/qgsjet-II-04.hpp>

#include <corsika/framework/random/RNGManager.hpp>

#include <iostream>
#include <random>

datadir::datadir(const std::string& dir) {
  if (dir.length() > 130) {
    std::cerr << "QGSJetII error, will cut datadir \"" << dir
              << "\" to 130 characters: " << std::endl;
  }
  int i = 0;
  for (i = 0; i < std::min(130, int(dir.length())); ++i) data[i] = dir[i];
  data[i + 0] = ' ';
  data[i + 1] = '\0';
}

double qgran_(int&) {
  static corsika::RNG& rng =
      corsika::RNGManager::GetInstance().GetRandomStream("qgran");

  std::uniform_real_distribution<double> dist;
  return dist(rng);
}

void lzmaopenfile_(const char*, int) {}
void lzmaclosefile_() {}
void lzmafillarray_(const double&, const int&) {}
