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
//#include <bits/stdc++.h>
//#include <corsika/media/IRefractiveIndexModel.hpp>
//
//namespace corsika {
//
//  template <typename T>
//  template <typename... Args>
//  ExponentialRefractiveIndex<T>::ExponentialRefractiveIndex(double const n0,
//                             InverseLengthType const lambda, Args&&... args)
//      : T(std::forward<Args>(args)...)
//      , n_0(n0)
//      , lambda_(lambda) {}
//
//  template <typename T>
//  double ExponentialRefractiveIndex<T>::getRefractiveIndex(Point const& point) const {
//  //TODO: THIS METHOD CURRENTLY ONLY USES THE Z-COORDINATE.
//  //NEED TO THINK IT FOR FUTURE WORK ON ARBITRARY GEOMETRIES.
//  return n_0 * exp((-lambda_) * point.getCoordinates().getZ());
//  }
//
//} // namespace corsika
