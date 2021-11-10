/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <boost/math/special_functions/legendre.hpp>
#include <cmath>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/utility/CorsikaData.hpp>
#include <fstream>
#include <iostream>
using namespace std;

double factorial(int n) {
  double factorial = 1;
  if (n < 0) {
    CORSIKA_LOG_ERROR("Factorial of a negative number does not exist.");
    abort();
  }
  else {
    for(int i = 1; i <= n; ++i) {
      factorial *= i;
    }
  }
  return factorial;
}

namespace corsika {
  MagneticFieldVector get_wmm(const CoordinateSystemPtr Cs, const double year, const LengthType altitude, const double latitude, const double longitude) {
    if (year < 2020 || year > 2025) {
      CORSIKA_LOG_ERROR("Year has to be between 2020 and 2025.");
      abort();
    }
    if (altitude < -10_km || altitude > 3000_km) {
      CORSIKA_LOG_WARN("Altitude should be between -10_km and 3000_km.");
    }
    if (latitude < -90 || latitude > 90) {
      CORSIKA_LOG_ERROR("Latitude has to be between -90 and 90 degree.");
      abort();
    } else if (latitude < -82 || latitude > 82) {
      CORSIKA_LOG_WARN("Latitude is close to the poles.");
    }
    if (longitude < -180 || longitude > 180) {
      CORSIKA_LOG_ERROR("Longitude has to be between -180 and 180 degree.");
      abort();
    }
    
    double lat_geo = latitude * constants::pi / 180;
    double lon = longitude * constants::pi / 180;
  
    // Transform into spherical coordinates
    const LengthType A = 6378137_m;
    const double f = 1 / 298.257223563;
    const double e_squared = f * (2 - f);
    LengthType R_c = A / sqrt(1 - e_squared * pow(sin(lat_geo), 2));
    LengthType p = (R_c + altitude) * cos(lat_geo);
    LengthType z = sin(lat_geo) * (altitude + R_c * (1 - e_squared));
    LengthType r = sqrt(p * p + z * z);
    double lat_sph = asin(z / r);
    
    const int length = 90;
    double epoch, g[length], h[length], g_dot[length], h_dot[length];
    char model_name[7];
    char release_date[10];
    int n[length], m[length];
    ifstream infile;
    
    // Read in coefficients
    boost::filesystem::path const path = corsika::corsika_data("GeoMag/WMM.COF");
    char mdfile[path.generic_string().size() + 1];
    strcpy(mdfile, path.generic_string().c_str());    //name and path of model file
    infile.open(mdfile);
    // Exit if file opening failed
    if (!infile.is_open()){
      CORSIKA_LOG_ERROR("Failed opening WMM.COF.");
      abort();
    }
    
    infile >> epoch >> model_name >> release_date;
    for (int i = 0; i < length; i++) {
      infile >> n[i] >> m[i] >> g[i] >> h[i] >> g_dot[i] >> h_dot[i];
      // Time interpolation
      g[i] = g[i] + (year - epoch) * g_dot[i];
      h[i] = h[i] + (year - epoch) * h_dot[i];
    }
    infile.close();
    
    double legendre, next_legendre, derivate_legendre;
    double magneticfield[3] = {0, 0, 0};
    
    for (int j = 0; j < length; j++) {
      // cout << assoc_legendre(n[0], m[0], sin(lat_sph)) << endl;
      legendre = boost::math::legendre_p(n[j], m[j], sin(lat_sph));
      next_legendre = boost::math::legendre_p(n[j] + 1, m[j], sin(lat_sph));
      
      // Schmidt semi-normalization and Condon-Shortley phase term
      if (m[j] > 0) {
        legendre *= sqrt(2 * factorial(n[j] - m[j]) / factorial(n[j] + m[j])) * pow(-1, m[j]);
        next_legendre *= sqrt(2 * factorial(n[j] + 1 - m[j]) / factorial(n[j] + 1 + m[j])) * pow(-1, m[j]);
      }
      derivate_legendre = (n[j] + 1) * tan(lat_sph) * legendre - sqrt(pow(n[j] + 1, 2) - pow(m[j], 2)) / cos(lat_sph) * next_legendre;
      
      magneticfield[0] += pow(constants::EarthRadius::Mean / r, n[j] + 2) * (g[j] * cos(m[j] * lon) + h[j] * sin(m[j] * lon)) * derivate_legendre;
      magneticfield[1] += pow(constants::EarthRadius::Mean / r, n[j] + 2) * m[j] * (g[j] * sin(m[j] * lon) - h[j] * cos(m[j] * lon)) * legendre;
      magneticfield[2] += (n[j] + 1) * pow(constants::EarthRadius::Mean / r, n[j] + 2) * (g[j] * cos(m[j] * lon) + h[j] * sin(m[j] * lon)) * legendre;
    }
    magneticfield[0] *= -1;
    magneticfield[1] /= cos(lat_sph);
    magneticfield[2] *= -1;
    
    // Transform back into geodetic coordinates
    double magneticfield_geo[3];
    magneticfield_geo[0] = magneticfield[0] * cos(lat_sph - lat_geo) - magneticfield[2] * sin(lat_sph - lat_geo);
    magneticfield_geo[1] = magneticfield[1];
    magneticfield_geo[2] = magneticfield[0] * sin(lat_sph - lat_geo) + magneticfield[2] * cos(lat_sph - lat_geo);
    
    return MagneticFieldVector{Cs, magneticfield_geo[0] * 1_nT, magneticfield_geo[1] * 1_nT, magneticfield_geo[2] * -1_nT};
  }
}