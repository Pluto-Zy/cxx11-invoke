#include <type_traits>
#include <utility>

namespace cxx11_invoke {
  namespace detail {
    /*
    * Available in C++11:
    *   std::remove_cv
    *   std::remove_reference
    *   std::true_type and std::false_type
    *   std::is_base_of
    *   std::is_member_function_pointer and std::is_member_object_pointer
    *   std::enable_if
    *   std::forward
    *   std::integral_constant
    *   std::is_void
    *   std::is_convertible
    *   ...
    */

    // remove_cvref
    template<class Type>
    struct remove_cvref : std::remove_cv<typename std::remove_reference<Type>::type> { };

    template<class Type>
    struct is_specialize_of_reference_wrapper_impl : std::false_type { };
    template<class Type>
    struct is_specialize_of_reference_wrapper_impl<std::reference_wrapper<Type>> : std::true_type { };

    // check_base
    template<class Base, class Derived>
    struct check_base : std::is_base_of<Base, typename std::remove_reference<Derived>::type> { };

    // check_specialize
    template<class Type>
    struct check_specialize : is_specialize_of_reference_wrapper_impl<typename remove_cvref<Type>::type> { };

    template<class Func,
      bool is_mem_func = std::is_member_function_pointer<typename remove_cvref<Func>::type>::value,
      bool is_data_mem = std::is_member_object_pointer<typename remove_cvref<Func>::type>::value>
    struct invoke_impl;

    // call of pointer to member function
    template<class F>
    struct invoke_impl<F, true, false> {
      template<class Func, class Class, class FirstArg, class... Args,
        typename std::enable_if<check_base<Class, FirstArg>::value, int>::type = 0>
      static constexpr auto call(Func Class::* func, FirstArg&& arg1, Args&&... args)
        noexcept(noexcept((std::forward<FirstArg>(arg1).*func)(std::forward<Args>(args)...))) ->
          decltype((std::forward<FirstArg>(arg1).*func)(std::forward<Args>(args)...)) {
        return (std::forward<FirstArg>(arg1).*func)(std::forward<Args>(args)...);
      }

      template<class Func, class Class, class FirstArg, class... Args,
        typename std::enable_if<check_specialize<FirstArg>::value, int>::type = 0>
      static constexpr auto call(Func Class::* func, FirstArg&& arg1, Args&&... args)
        noexcept(noexcept((arg1.get().*func)(std::forward<Args>(args)...))) ->
          decltype((arg1.get().*func)(std::forward<Args>(args)...)) {
        return (arg1.get().*func)(std::forward<Args>(args)...);
      }

      template<class Func, class Class, class FirstArg, class... Args,
        typename std::enable_if<!check_base<Class, FirstArg>::value && !check_specialize<FirstArg>::value, int>::type = 0>
      static constexpr auto call(Func Class::* func, FirstArg&& arg1, Args&&... args)
        noexcept(noexcept(((*std::forward<FirstArg>(arg1)).*func)(std::forward<Args>(args)...))) ->
          decltype(((*std::forward<FirstArg>(arg1)).*func)(std::forward<Args>(args)...)) {
        return ((*std::forward<FirstArg>(arg1)).*func)(std::forward<Args>(args)...);
      }
    };

    // call of pointer of data member
    template<class F>
    struct invoke_impl<F, false, true> {
      template<class Func, class Class, class FirstArg,
        typename std::enable_if<check_base<Class, FirstArg>::value, int>::type = 0>
      static constexpr auto call(Func Class::* func, FirstArg&& arg1)
        noexcept(noexcept(std::forward<FirstArg>(arg1).*func)) ->
          decltype(std::forward<FirstArg>(arg1).*func) {
        return std::forward<FirstArg>(arg1).*func;
      }

      template<class Func, class Class, class FirstArg,
        typename std::enable_if<check_specialize<FirstArg>::value, int>::type = 0>
      static constexpr auto call(Func Class::* func, FirstArg&& arg1)
        noexcept(noexcept(arg1.get().*func)) ->
          decltype(arg1.get().*func) {
        return arg1.get().*func;
      }

      template<class Func, class Class, class FirstArg,
        typename std::enable_if<!check_base<Class, FirstArg>::value && !check_specialize<FirstArg>::value, int>::type = 0>
      static constexpr auto call(Func Class::* func, FirstArg&& arg1)
        noexcept(noexcept((*std::forward<FirstArg>(arg1)).*func)) ->
          decltype((*std::forward<FirstArg>(arg1)).*func) {
        return (*std::forward<FirstArg>(arg1)).*func;
      }
    };

    // call of function object
    template<class F>
    struct invoke_impl<F, false, false> {
      template<class Func, class... Args>
      static constexpr auto call(Func&& func, Args&&... args)
        noexcept(noexcept(std::forward<Func>(func)(std::forward<Args>(args)...))) ->
          decltype(std::forward<Func>(func)(std::forward<Args>(args)...)) {
        return std::forward<Func>(func)(std::forward<Args>(args)...);
      }
    };

    // implement of std::void_t in C++17
    template<class...>
    struct void_t {
      using type = void;
    };

    template<class Void, class Func, class... Args>
    struct is_invocable_impl : std::false_type { };

    template<class Func, class... Args>
    struct is_invocable_impl<
      typename void_t<decltype(invoke_impl<Func>::call(std::declval<Func>(), std::declval<Args>()...))>::type, 
        Func, Args...> : std::true_type { };

    template<class Int, class Func, class... Args>
    struct invoke_result_impl;

    template<class Func, class... Args>
    struct invoke_result_impl<
      typename std::enable_if<is_invocable_impl<void, Func, Args...>::value, int>::type,
        Func, Args...> {
      using type = decltype(invoke_impl<Func>::call(std::declval<Func>(), std::declval<Args>()...));
    };
  }

  template<class Func, class... Args>
  struct invoke_result : detail::invoke_result_impl<int, Func, Args...> { };

  template<class Func, class... Args>
  struct is_invocable : detail::is_invocable_impl<void, Func, Args...> { };

  template<class Ret, class Func, class... Args>
  struct is_invocable_r : std::integral_constant<bool, 
    is_invocable<Func, Args...>::value && 
      (std::is_void<Ret>::value || std::is_convertible<typename invoke_result<Func, Args...>::type, Ret>::value)> { };

  template<class Func, class... Args>
  constexpr typename invoke_result<Func, Args...>::type
    invoke(Func&& func, Args&&... args)
      noexcept(noexcept(detail::invoke_impl<Func>::call(std::forward<Func>(func), std::forward<Args>(args)...))) {
    return detail::invoke_impl<Func>::call(std::forward<Func>(func), std::forward<Args>(args)...);
  }
}