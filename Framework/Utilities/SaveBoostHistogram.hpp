#include <corsika/third_party/cnpy/cnpy.hpp>

#include <boost/histogram.hpp>

#include <functional>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace corsika {
  namespace utl {

    template <typename hist_type>
    void save_hist(hist_type const& h, std::string const& filename) {
      auto const rank = h.rank();
      auto const size = h.size();
      using value_type = typename hist_type::value_type;

      std::vector<size_t> axes_dims;
      axes_dims.reserve(rank);

      std::vector<char> axis_types;
      axis_types.reserve(rank);

      for (int i = 0; i < rank; ++i) {
        auto const& ax = h.axis(i);
        int const has_underflow = (ax.options() & 0x01) ? 1 : 0;
        int const has_overflow = (ax.options() & 0x02) ? 1 : 0;

        axes_dims.emplace_back(ax.size() + has_underflow + has_overflow);

        if (ax.continuous()) {
          axis_types.push_back('c');
          std::vector<double> ax_edges;
          ax_edges.reserve(ax.size());

          for (int j = 0; j <= ax.size(); ++j) { ax_edges.push_back(ax.bin(j).lower()); }

          cnpy::npz_save(filename, std::string{"binedges_"} + std::to_string(i),
                         ax_edges.data(), {ax_edges.size()}, "a");
        } else {
          axis_types.push_back('d');
          std::vector<int64_t> bins; // we assume that discrete axes have integer bins
          bins.reserve(ax.size());

          for (int j = 0; j < ax.size(); ++j) { bins.push_back(ax.bin(j).lower()); }

          cnpy::npz_save(filename, std::string{"bins_"} + std::to_string(i), bins.data(),
                         {bins.size()}, "a");
        }

        cnpy::npz_save(filename, std::string{"axistypes"}, axis_types.data(),
                       {axis_types.size()}, "a");
      }

      auto const prod_axis_size =
          std::accumulate(axes_dims.cbegin(), axes_dims.cend(), 1, std::multiplies<>());
      auto temp = std::make_unique<value_type[]>(prod_axis_size);

      std::cout << "rank = " << rank << std::endl;
      std::cout << "size = " << size << std::endl;
      std::cout << "prod_axis_size = " << prod_axis_size << std::endl;

      // reduce multi-dim. to 1-dim, row-major (i.e., last axis index is contiguous in
      // memory) take special care of underflow bins, which have -1 as index and thus need
      // to be shifted by +1

      for (auto&& x : indexed(h, boost::histogram::coverage::all)) {
        int const offset_underflow = (h.axis(0).options() & 0x01) ? 1 : 0;
        auto p = x.index(0) + offset_underflow; // 1-d-index

        for (size_t axis_index = 1; axis_index < rank; ++axis_index) {
          int const offset_underflow = (h.axis(axis_index).options() & 0x01) ? 1 : 0;
          auto k = x.index(axis_index) + offset_underflow;
          p = k + p * axes_dims.at(axis_index);
          std::cout << " " << x.index(axis_index);
        }

        temp[p] = *x;
      }

      cnpy::npz_save(filename, "data", temp.get(), axes_dims, "a");
      // In Python this array can directly be assigned to a histogram view if that
      // histogram has its axes correspondingly: hist.view(flow=True)[:] = file['data']
    }
  } // namespace utl
} // namespace corsika
