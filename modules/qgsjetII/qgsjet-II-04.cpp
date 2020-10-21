#include <qgsjet-II-04.hpp>

#include <iostream>

datadir::datadir(const std::string& dir) {
  if (dir.length() > 130) {
    std::cerr << "QGSJetII error, will cut datadir \"" << dir
              << "\" to 130 characters: " << std::endl;
  }
  int i = 0;
  for (i = 0; i < std::min(130, int(dir.length())); ++i) data[i] = dir[i];
  data[i + 0] = ' ';
  data[i + 1] = '\0';
}


/**
   @function qgran

   link to random number generation
 */
double qgran_(int&) { return qgsjetII::rndm_interface(); } 

