/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/CoordinateSystem.h>
#include <stdexcept>

using namespace corsika::geometry;

/**
 * returns the transformation matrix necessary to transform primitives with coordinates
 * in \a pFrom to \a pTo, e.g.
 * \f$ \vec{v}^{\text{(to)}} = \mathcal{M} \vec{v}^{\text{(from)}} \f$
 * (\f$ \vec{v}^{(.)} \f$ denotes the coordinates/components of the component in
 * the indicated CoordinateSystem).
 */
EigenTransform CoordinateSystem::GetTransformation(CoordinateSystem const& pFrom,
                                                   CoordinateSystem const& pTo) {
  CoordinateSystem const* a{&pFrom};
  CoordinateSystem const* b{&pTo};
  CoordinateSystem const* commonBase{nullptr};

  while (a != b && b != nullptr) {
    a = &pFrom;

    while (a != b && a != nullptr) { a = a->GetReference(); }

    if (a == b) break;

    b = b->GetReference();
  }

  if (a == b && a != nullptr) {
    commonBase = a;

  } else {
    throw std::runtime_error("no connection between coordinate systems found!");
  }

  EigenTransform t = EigenTransform::Identity();
  auto* p = &pFrom;

  while (p != commonBase) {
    t = p->GetTransform() * t;
    p = p->GetReference();
  }

  p = &pTo;

  while (p != commonBase) {
    t = t * p->GetTransform().inverse(Eigen::TransformTraits::Isometry);
    p = p->GetReference();
  }

  return t;
}
