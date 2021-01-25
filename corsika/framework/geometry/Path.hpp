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

  /**
   * This class represents a (potentially) curved path between two
   * points using N >= 1 straight-line segments.
   */
  class Path {
    std::deque<Point> points_;     ///< The points that make up this path.
    LengthType length_= LengthType::zero(); ///< The length of the path.
  public:
    /**
     * Create a Path with a given starting Point.
     */
    Path(Point const& point) {
      points_.push_front(point);
    }

    /**
     * Initialize a Path from an existing collection of Points.
     */
    Path(std::deque<Point> const& points)
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

    /**
     * Add a new Point to the end of the path.
     */
    void AddToEnd(Point const& point) {
      length_ += (point - points_.back()).getNorm();
      points_.push_back(point);
    }

    /**
     * Remove a point from the end of the path.
     */
    void RemoveFromEnd() {
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

    /**
     * Get the total length of the path.
     */
    LengthType GetLength() const {
      return length_;
    }

    /**
     * Get the starting point of the path.
     */
    Point GetStart() const {
      return points_.front();
    }

    /**
     * Get the end point of the path.
     */
    Point GetEnd() const {
      return points_.back();
    }

    /**
     * Get a specific point of the path.
     */
    Point GetPoint(std::size_t const index) const {
      return points_.at(index);
    }

    /**
     * Return an iterator to the start of the Path.
     */
    auto begin() { return points_.begin(); }

    /**
     * Return an iterator to the end of the Path.
     */
    auto end() { return points_.end(); }

    /**
     * Get the number of steps in the path.
     *
     * This is one less than the number of points that
     * defines the path.
     */
    int GetNSegments() const { return points_.size() - 1; }

  };  // class Path

} // namespace corsika