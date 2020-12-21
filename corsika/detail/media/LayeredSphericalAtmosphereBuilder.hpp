#pragma once

namespace corsika {

  namespace detail {

    struct NoExtraModelInner {};

    template <typename M>
    struct NoExtraModel {};

    template <template <typename> typename M>
    struct has_extra_models : std::true_type {};

    template <>
    struct has_extra_models<NoExtraModel> : std::false_type {};

  } // namespace detail

}
