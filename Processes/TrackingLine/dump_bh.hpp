#pragma once

#include <boost/histogram.hpp> 
#include <sstream>
#include <string>
#include <vector>

/*
  To save one 1D boost::histogram in C++, include this file and run:

  dump_bh_1d("file_name.json", hist);

  Note that the file_name.json will be overwritten, so use a new file for each histogram! or use:

  std::ofstream file1("test_hist.json");
  dump_bh(file1, h1);
  file << ",\n";
  dump_bh(file1, h2);
  file << ",\n";
  dump_bh(file1, h3);
  file1.close();


  In python, just read the json and plot this histogram with matplotib
  
  for 1D histogram:

  import json
  import matplotlib.pyplot as plt

  h1=json.load(open("test_hist_direct.json"))
  print (h1)
  plt.hist(h1['bins'][:-1], bins=h1['bins'], weights=h1['data'])
  plt.show()


  for 2D histogram (needs some list/array gymnastics):

  import json
  import matplotlib.pyplot as plt
  import numpy as np

  h2=json.load(open("test_hist_2D_direct.json"))
  print (h2)
  xx,yy = np.meshgrid(h2['xbins'][:-1], h2['ybins'][:-1])
  plt.hist2d([i for s in xx for i in s], [i for s in yy for i in s], 
             bins=[h2['xbins'],h2['ybins']], weights=h2['data'])
  plt.show()


  Do not forget to add axis titles, legend, etc. 
 */


template<typename T>
void dump_bh_2d(std::ostream& os, T& h)
{
  // determine number of axes (rank) and axes sizes
  const int rank = 2;
  std::vector<int> axis_size(rank);
  for (unsigned int i=0; i<rank; ++i)
    axis_size[i] = h.axis(i).size();

  // start dump json object
  os << "{\n";
  os << "  \"rank\" : " << rank << ",\n";
  os << "  \"size\" : [";
  bool first = true;
  for (auto s : axis_size) {
    os << (first?"":", ") << s;
    first = false;
  }
  os << "],\n";

  // bins-x
  os << "  \"xbins\" : [";
  double lastx = 0;
  for (int i=0; i<h.axis(0).size(); ++i) {
    os << h.axis(0).bin(i).lower() << ", ";
    lastx = h.axis(0).bin(i).upper();
  }
  os << lastx << "],\n";

  // bins-y
  os << "  \"ybins\" : [";
  double lasty = 0;
  for (int i=0; i<h.axis(1).size(); ++i) {
    os << h.axis(1).bin(i).lower() << ", ";
    lasty = h.axis(1).bin(i).upper();
  }
  os << lasty << "],\n";

  // binned data
  os << "  \"data\" : [";
  first = true;
  for (auto x : boost::histogram::indexed(h)) {
    os << (first?"":", ") <<  *x;
    first = false;
  }
  os << "  ]\n";
  os << "}\n";
}




template<typename T>
void dump_bh_1d(std::ostream& os, T& h)
{
  const int rank = 1;
  // determine number of axes (rank) and axes sizes
  std::vector<int> axis_size(rank);
  for (unsigned int i=0; i<rank; ++i)
    axis_size[i] = h.axis(i).size();

  // start dump json object
  os << "{\n";
  os << "  \"rank\" : 1,\n";
  os << "  \"size\" : [";
  bool first = true;
  for (auto s : axis_size) {
    os << (first?"":", ") << s << "],\n";
    first = false;
  }

  // underflow data
  os << "  \"underflow\" : [";
  os << h.at(boost::histogram::axis::option::underflow);
  os << "],\n";

  // overflow data
  os << "  \"overflow\" : [";
  os << h.at(boost::histogram::axis::option::overflow);
  os << "],\n";

  // bins
  os << "  \"bins\" : [";
  double last = 0;
  for (auto x : boost::histogram::indexed(h)) {
    os << x.bin().lower() << ", ";
    last = x.bin().upper();
  }
  os << last << "],\n";

  // data of bins
  os << "  \"data\" : [";
  first = true;
  for (auto x : boost::histogram::indexed(h)) {
    os << (first?"":", ") <<  *x;
    first = false;
  }
  os << "  ]\n";
  os << "}\n";
}




template<typename T>
void dump_bh(std::ostream& os, T& h)
{
  // determine number of axes (rank) and axes sizes
  const int rank = h.rank();
  switch (rank) {
  case 1:
    dump_bh_1d(os, h);
    break;
  case 2:
    dump_bh_2d(os, h);
    break;
  default:
    {
      throw std::runtime_error("multi dim hist not implemented");
    }
    break;
  }
}

template<typename T>
void dump_bh(const char* fname, T& h)
{
  std::ofstream file(fname);
  dump_bh(file, h);
  file.close();
}

