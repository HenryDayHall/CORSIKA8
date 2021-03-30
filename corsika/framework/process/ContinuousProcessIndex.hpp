/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  /**
     @ingroup Processes

     To index individual processes (continuous processes) inside a
     ProcessSequence.
    
   **/

  class ContinuousProcessIndex {
  public:
    ContinuousProcessIndex()
        : id_(-1) {} // default
    ContinuousProcessIndex(int const id)
        : id_(id) {}
    void setIndex(int const id) { id_ = id; }
    int getIndex() const { return id_; }
    bool operator==(ContinuousProcessIndex const v) const { return id_ == v.id_; }
    bool operator!=(ContinuousProcessIndex const v) const { return id_ != v.id_; }

  private:
    int id_;
  };

} // namespace corsika
