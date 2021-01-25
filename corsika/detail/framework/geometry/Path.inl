///*
// * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
// *
// * This software is distributed under the terms of the GNU General Public
// * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
// * the license.
// */
//
//#pragma once
//
//#include <deque>
//#include <corsika/framework/geometry/Point.hpp>
//
//namespace corsika {
//
//  inline void AddToEnd(Point const& point) {
//    length_ += (point - points_.back()).getNorm();
//    points_.push_back(point);
//  }
//
//
//  inline void RemoveFromEnd() {
//    auto lastpoint_ = points_.back();
//    points_.pop_back();
//    int dequesize_ = points_.size();
//    if (dequesize_ == 0 || dequesize_ == 1) {
//      length_ = LengthType::zero();
//    }
//    else if (dequesize_ == 2) {
//      length_ = (points_.back() - points_.front()).getNorm();
//    }
//    else { length_ -= (lastpoint_ - points_.back()).getNorm(); }
//  }
//
//
//  inline LengthType GetLength() const {
//    return length_;
//  }
//
//
//  inline Point GetStart() const {
//    return points_.front();
//  }
//
//
//  inline Point GetEnd() const {
//    return points_.back();
//  }
//
//
//  inline Point GetPoint(std::size_t const index) const {
//    return points_.at(index);
//  }
//
//
//
//  inline int GetNSegments() const { return points_.size() - 1; }
//
//} // namespace corsika