/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef CORSIKA_CORSIKAFENV_H
#define CORSIKA_CORSIKAFENV_H

#include <cfenv>

/*
 * Same declaration of function as provided in GLIBC
 * Repetition allowed in the case where cfenv defines the functions already, no clash.
 */
extern "C" {

int feenableexcept(int excepts);
int fedisableexcept(int excepts);
}

#endif // CORSIKA_CORSIKAFENV_H
