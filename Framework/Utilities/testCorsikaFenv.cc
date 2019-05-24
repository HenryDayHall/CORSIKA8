/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/utl/CorsikaFenv.h>

#include <cmath>
#include <csignal>
#include <iostream>

extern "C" {
static void handle_fpe(int /*signo*/) { exit(0); }
}

int main() {
  feenableexcept(FE_ALL_EXCEPT);
  signal(SIGFPE, handle_fpe);

  std::cout << std::log(0.) << std::endl;

  exit(1);
}