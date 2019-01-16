/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 *
 * Test code for cmake to check if feenableexcept exists in cfenv
 *
 * \author Lukas Nellen
 * \date 15 Jan 2019
 *
 */

#include <cfenv>

int
main()
{
  feenableexcept(FE_ALL_EXCEPT);
  return 0;
}