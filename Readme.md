# cxx11-invoke

This is my attempt to use **C++11** to implement `std::invoke`.

### Introduction

We can use `std::invoke` to invoke *callable* objects (not just *function object*s). which additionally includes **pointers to member function** and **pointers to data member**.

Although `std::invoke` was added to the STL in C++17, we can still use STL facilities such as `std::function` or `std::bind` to call callable objects. All I did was just implement `std::invoke` using the language rules of C++11.

### Usage

All APIs are implemented in the header file. You just need to add `src/invoke.hpp` to your project, and include it in the source file where you want to use it:

```c++
#include "invoke.hpp"	// You must use the correct path.
```

You can use it just like `std::invoke`. For example, 

```c++
#include <string>
#include <iostream>
#include "invoke.hpp"

std::string s;
// invoke pointer to data member
std::cout << cxx11_invoke::invoke(&std::string::size, s);	
```

All the facilities you can use are listed below.

### Library facilities

1. `cxx11_invoke::invoke`: invoke the *callable object* `func` with the parameters `args`.

   ```c++
   template<class Func, class... Args>
   constexpr typename cxx11_invoke::invoke_result<Func, Args...>::type
   invoke(Func&& func, Args&&... args) noexcept(...);
   ```

   Refer to the standard requirements of [`std::invoke`]([std::invoke, std::invoke_r - cppreference.com](https://en.cppreference.com/w/cpp/utility/functional/invoke)) and [*Callable*]([C++ named requirements: Callable - cppreference.com](https://en.cppreference.com/w/cpp/named_req/Callable)).

2. `cxx11_invoke::invoke_result`: Deduces the return type of an [INVOKE expression](https://en.cppreference.com/w/cpp/named_req/Callable) at compile time.

   ```c++
   template<class Func, class... Args>
   struct invoke_result;
   ```

   Refer to the standard requirements of [`std::invoke_result`]([std::result_of, std::invoke_result - cppreference.com](https://en.cppreference.com/w/cpp/types/result_of)).

3. `cxx11_invoke::is_invocable`:  Determines whether `Func` can be invoked with the arguments `Args...`. 

   `cxx11_invoke::is_invocable_r`: Determines whether `Func` can be invoked with the arguments `Args...` to yield a result that is convertible to `Ret`.

   ```c++
   template<class Func, class... Args>
   struct is_invocable;
   
   template<class Ret, class Func, class... Args>
   struct is_invocable_r;
   ```

   Refer to the [standard requirements](https://en.cppreference.com/w/cpp/types/is_invocable) of `std::is_invocable` and `std::is_invocable_r`.

### About `noexcept`

Before C++17, `noexcept` is *not* a part of the function type. I didn't provide `is_nothrow_invocable`, because the type of the callable object does not contain `noexcept` information. But I still added the `noexcept` specifier to `cxx11_invoke::invoke`, which is just for completeness.