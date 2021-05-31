/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2021 Antonio Augusto Alves Junior
 *
 *   This file is part of RandomIterator.
 *
 *   RandomIterator is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   RandomIterator is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RandomIterator.  If not, see <http://www.gnu.org/licenses/>.
 *
 *---------------------------------------------------------------------------*/

/*
 * RandomIterator.h
 *
 *  Created on: 21/02/2021
 *      Author: Antonio Augusto Alves Junior
 */

#pragma once

//  This is the only RandomIterator header that is guaranteed to
//  change with every RandomIterator release.
//
//  RandomIterator_VERSION % 100 is the sub-minor version
//  RandomIterator_VERSION / 100 % 1000 is the minor version
//  RandomIterator_VERSION / 100000 is the major version
//
//  Because this header does not #include <RandomIterator/detail/Config.h>,
//  it is the only RandomIterator header that does not cause
//  RandomIterator_HOST_SYSTEM and RandomIterator_DEVICE_SYSTEM to be defined.

/*! \def RandomIterator_VERSION
 *  \brief The preprocessor macro \p RandomIterator_VERSION encodes the version
 *         number of the RandomIterator.
 *
 *         <tt>RandomIterator_VERSION % 100</tt> is the patch version.
 *         <tt>RandomIterator_VERSION / 100 % 1000</tt> is the feature version.
 *         <tt>RandomIterator_VERSION / 100000</tt> is the major version.
 */
#define RandomIterator_VERSION 100001


/*! \def RandomIterator_MAJOR_VERSION
 *  \brief The preprocessor macro \p RandomIterator_MAJOR_VERSION encodes the
 *         major version number of RandomIterator.
 */
#define RandomIterator_MAJOR_VERSION     (RandomIterator_VERSION / 100000)

/*! \def RandomIterator_MINOR_VERSION
 *  \brief The preprocessor macro \p RandomIterator_MINOR_VERSION encodes the
 *         minor version number of RandomIterator.
 */
#define RandomIterator_MINOR_VERSION     (RandomIterator_VERSION / 100 % 1000)

/*! \def RandomIterator_PATCH_NUMBER
 *  \brief The preprocessor macro \p RandomIterator_PATCH_NUMBER encodes the
 *         patch number of the RandomIterator library.
 */
#define RandomIterator_PATCH_NUMBER  0



// Declare these namespaces here for the purpose of Doxygenating them

/*!
 *  \brief \p random_iterator is the top-level namespace which contains all RandomIterator
 *         functions and types.
 */
namespace random_iterator{ }


