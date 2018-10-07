
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/CoordinateSystem.h>

using namespace corsika::geometry;

EigenTransform CoordinateSystem::GetTransformation(CoordinateSystem const& c1,
                                                   CoordinateSystem const& c2) {
  CoordinateSystem const* a{&c1};
  CoordinateSystem const* b{&c2};
  CoordinateSystem const* commonBase{nullptr};

  while (a != b && b != nullptr) {
    a = &c1;

    while (a != b && a != nullptr) { a = a->GetReference(); }

    if (a == b) break;

    b = b->GetReference();
  }

  if (a == b && a != nullptr) {
    commonBase = a;

  } else {
    throw std::string("no connection between coordinate systems found!");
  }

  EigenTransform t = EigenTransform::Identity();

  auto* p = &c1;

  while (p != commonBase) {
    t = p->GetTransform() * t;
    p = p->GetReference();
  }

  p = &c2;

  while (p != commonBase) {
    t = p->GetTransform().inverse(Eigen::TransformTraits::Isometry) * t;
    p = p->GetReference();
  }

  return t;
}
