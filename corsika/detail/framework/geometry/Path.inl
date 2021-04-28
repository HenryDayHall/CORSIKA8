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

namespace corsika {

  Path::Path(Point const& point) {
    points_.push_front(point);
  }

  Path::Path(std::deque<Point> const& points)
      : points_(points) {
    int dequesize_ = points.size();
    if (dequesize_ == 0 || dequesize_ == 1) {
      length_ = LengthType::zero();
    }
    else if (dequesize_ == 2) {
      length_ = (points.back() - points.front()).getNorm();
    }
    else {
      for (auto point = points.begin(); point !=  points.end() - 1; ++point) {
        auto point_next = *(point+1);
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
    auto lastpoint_ = points_.back();
    points_.pop_back();
    int dequesize_ = points_.size();
    if (dequesize_ == 0 || dequesize_ == 1) {
      length_ = LengthType::zero();
    }
    else if (dequesize_ == 2) {
      length_ = (points_.back() - points_.front()).getNorm();
    }
    else { length_ -= (lastpoint_ - points_.back()).getNorm(); }
  }

  inline LengthType Path::getLength() const {
    return length_;
  }

  inline Point Path::getStart() const {
    return points_.front();
  }

  inline Point Path::getEnd() const {
    return points_.back();
  }

  inline Point Path::getPoint(std::size_t const index) const {
    return points_.at(index);
  }

  inline auto Path::begin() { return points_.begin(); }

  inline auto Path::end() { return points_.end(); }

  inline int Path::getNSegments() const { return points_.size() - 1; }

} // namespace corsika