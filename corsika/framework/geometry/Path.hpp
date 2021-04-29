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
    std::deque<Point> points_;               ///< The points that make up this path.
    LengthType length_ = LengthType::zero(); ///< The length of the path.
  public:
    /**
     * Create a Path with a given starting Point.
     */
    Path(Point const& point);

    /**
     * Initialize a Path from an existing collection of Points.
     */
    Path(std::deque<Point> const& points);

    /**
     * Add a new Point to the end of the path.
     */
    inline void addToEnd(Point const& point);

    /**
     * Remove a point from the end of the path.
     */
    inline void removeFromEnd();

    /**
     * Get the total length of the path.
     */
    inline LengthType getLength() const;

    /**
     * Get the starting point of the path.
     */
    inline Point getStart() const;

    /**
     * Get the end point of the path.
     */
    inline Point getEnd() const;

    /**
     * Get a specific point of the path.
     */
    inline Point getPoint(std::size_t const index) const;

    /**
     * Return an iterator to the start of the Path.
     */
    inline auto begin();

    /**
     * Return an iterator to the end of the Path.
     */
    inline auto end();

    /**
     * Get the number of steps in the path.
     * This is one less than the number of points that
     * defines the path.
     */
    inline int getNSegments() const;

  }; // class Path

} // namespace corsika

#include <corsika/detail/framework/geometry/Path.inl>