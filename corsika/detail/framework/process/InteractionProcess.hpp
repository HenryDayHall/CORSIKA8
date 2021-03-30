#pragma once

namespace corsika {

  namespace detail {

    /**
       Helper traits class (partial) for static compile time checking.

       Note, this is a poor replacement for C++20 concepts... they are
       eagerly awaited!

       It defines the default body of a generic test function returning
       std::false_type.

       In addition it defines the pattern for class-method matching with a
       return type TReturn and function arguments TArgs... . Right now
       both method signatures, "const" and "not const", are matched.
     */
    template <typename TReturn, typename... TArgs>
    struct has_method_signature {

      template <class T>
      static std::true_type testSignature(TReturn (T::*)(TArgs&&...));

      template <class T>
      static std::true_type testSignature(TReturn (T::*)(TArgs&&...) const);

      template <class T>
      static std::false_type test(...);
    };

  } // namespace detail

  template <class TProcess, typename TReturn, typename... TArgs>
  struct has_method_doInteract : public detail::has_method_signature<TReturn, TArgs...> {

    using detail::has_method_signature<TReturn, TArgs...>::testSignature;

    template <class T>
    static decltype(testSignature(&T::template doInteraction<TArgs...>)) test(
        std::nullptr_t);

    template <class T>
    static decltype(testSignature(&T::doInteraction)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArgs>
  bool constexpr has_method_doInteract_v =
      has_method_doInteract<TProcess, TReturn, TArgs...>::value;

  template <class TProcess, typename TReturn, typename... TArgs>
  struct has_method_getInteractionLength
      : public detail::has_method_signature<TReturn, TArgs...> {

    using detail::has_method_signature<TReturn, TArgs...>::testSignature;

    template <class T>
    static decltype(testSignature(&T::template getInteractionLength<TArgs...>)) test(
        std::nullptr_t);

    template <class T>
    static decltype(testSignature(&T::getInteractionLength)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArgs>
  bool constexpr has_method_getInteractionLength_v =
      has_method_getInteractionLength<TProcess, TReturn, TArgs...>::value;

} // namespace corsika
