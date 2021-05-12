/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/detail/setup/SetupStack.hpp>

#include <array>
#include <memory>

namespace corsika::setup {

  // ---------------------------------------
  // this is the stack we use in C8 executables:

#ifdef WITH_HISTORY

#include <corsika/stack/history/HistoryStackExtension.hpp>
#include <corsika/stack/history/HistorySecondaryProducer.hpp>

  /*
   * the version with history
   */
  using Stack = detail::StackWithHistory;
  template <typename T1, template <typename> typename M2>
  using StackViewProducer = history::HistorySecondaryProducer<T1, M2>;

#else // WITH_HISTORY

  /*
   * the version without history
   */
  using Stack = detail::StackWithGeometry;
  template <typename T1, template <typename> typename M2>
  using StackViewProducer = DefaultSecondaryProducer<T1, M2>;

#endif

  // ---------------------------------------
  // this is the stackitertor (particle type) we use in C8 executables:

  /*
    See Issue 161

    unfortunately clang does not support this in the same way (yet) as
    gcc, so we have to distinguish here. If clang cataches up, we
    could remove the clang branch here and also in
    corsika::Cascade. The gcc code is much more generic and
    universal. If we could do the gcc version, we won't had to define
    StackView globally, we could do it with MakeView whereever it is
    actually needed. Keep an eye on this!
  */

#ifdef WITH_HISTORY

#if defined(__clang__)
  using StackView = SecondaryView<typename Stack::stack_data_type,
                                  // CHECK with CLANG: setup::Stack::MPIType>;
                                  detail::StackWithHistoryInterface, StackViewProducer>;
#elif defined(__GNUC__) || defined(__GNUG__)
  using StackView = MakeView<setup::Stack, StackViewProducer>::type;
#endif

#else // WITH_HISTORY

#if defined(__clang__)
  using StackView = SecondaryView<typename setup::Stack::stack_data_type,
                                  // CHECK with CLANG:
                                  // setup::Stack::MPIType>;
                                  setup::detail::StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
  using StackView = MakeView<setup::Stack>::type;
#endif

#endif // WITH_HISTORY

} // namespace corsika::setup
