/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <deque>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  Path::Path(Point const& point) { points_.push_front(point); }

  Path::Path(std::deque<Point> const& points)
      : points_(points) {
    int dequesize_ = points.size();
    if (dequesize_ == 0 || dequesize_ == 1) {
      length_ = LengthType::zero();
    } else if (dequesize_ == 2) {
      length_ = (points.back() - points.front()).getNorm();
    } else {
      for (auto point = points.begin(); point != points.end() - 1; ++point) {
        auto point_next = *(point + 1);
        auto point_now = *(point);
        length_ += (point_next - point_now).getNorm();
      }
    }
  }

  inline void Path::addToEnd(Point const& point) {
    length_ += (point - points_.back()).getNorm();
    points_.push_back(point);
  }

  inline void Path::removeFromEnd() {
    int const dequesize = points_.size();
    if (dequesize == 0) {
      length_ = LengthType::zero();
      return;
    }
    if (dequesize == 1) {
      length_ = LengthType::zero();
      return;
    }

    length_ -= distance(points_.back(), points_[dequesize - 2]);
    points_.pop_back();
  }

  inline LengthType Path::getLength() const { return length_; }

  inline Point const& Path::getStart() const { return points_.front(); }

  inline Point const& Path::getEnd() const { return points_.back(); }

  inline Point const& Path::getPoint(std::size_t const index) const {
    return points_.at(index);
  }

  inline Path::iterator Path::begin() { return points_.begin(); }
  inline Path::const_iterator Path::begin() const { return points_.cbegin(); }

  inline Path::iterator Path::end() { return points_.end(); }
  inline Path::const_iterator Path::end() const { return points_.cend(); }

  inline int Path::getNSegments() const { return points_.size() - 1; }

} // namespace corsika
