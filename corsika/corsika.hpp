/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

//  Usage:
//  to get the version X.YY.Z,
//  set CORSIKA_VERSION X0YY0Z
//
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