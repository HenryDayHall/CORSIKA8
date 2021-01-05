/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
  @file corsika.hpp

  The CORSIKA 8 air shower simulation framework.

  @mainpage Technical documentation of the CORSIKA 8 software framework

  Software documentatin and reference guide for the CORSIKA 8
  software framework for air shower simulations. CORSIKA 8 is developed
  at <a href="https://gitlab.ikp.kit.edu/AirShowerPhysics">https://gitlab.ikp.kit.edu</a>.
  If you got the code from somewhere else, consider to switch to the official development
  repository. If you want to report bugs, or want to suggest features or future
  development, please submit an "issue" on this gitlab server. We only accept Issues and
  discussion via our central development server https://gitlab.ikp.kit.edu.

  Write to corsika-devel@lists.kit.edu, or even register yourself at
  https://www.lists.kit.edu/sympa/info/corsika-devel to get in contact
  with other developers.

  For more information about the project, see @ref md_README.
  **/

/*! Usage:
 *  to get the version X.YY.Z,
 *  set CORSIKA_VERSION X0YY0Z
 */
#define CORSIKA_VERSION 800000

/*! \def CORSIKA_MAJOR_VERSION
 *  \brief The preprocessor macro \p CORSIKA_MAJOR_VERSION encodes the
 *         major version number of CORSIKA.
 */
#define CORSIKA_MAJOR_VERSION (CORSIKA_VERSION / 100000)

/*! \def CORSIKA_MINOR_VERSION
 *  \brief The preprocessor macro \p CORSIKA_MINOR_VERSION encodes the
 *         minor version number of CORSIKA.
 */
#define CORSIKA_MINOR_VERSION (CORSIKA_VERSION / 100 % 1000)

/*! \def CORSIKA_PATCH_NUMBER
 *  \brief The preprocessor macro \p CORSIKA_PATCH_NUMBER encodes the
 *         patch number of the CORSIKA library.
 */
#define CORSIKA_PATCH_NUMBER 0
