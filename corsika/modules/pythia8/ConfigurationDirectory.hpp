/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <string>

/**
 * \file Interaction.inl
 **/

///! Helper macros to convert definitions into strings
#define CORSIKA_STRINGIFY(x) #x
#define CORSIKA_TOSTRING(x) CORSIKA_STRINGIFY(x)

/**
 *
 * Location of the pythia XML directory with crucial input and config data:
 **/
static std::string const CORSIKA_Pythia8_XML_DIR =
    std::string(CORSIKA_Pythia8_PREFIX) // from cmake
    + std::string("/share/Pythia8/xmldoc/");
