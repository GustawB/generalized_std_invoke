#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <iostream>
#include <functional>
#include <utility>
#include <array>
#include <tuple>

//https://en.cppreference.com/w/cpp/utility/functional/bind_front

// Namespace containing all helper functions, naming based on the
// std::invoke documentation available here:
// https://en.cppreference.com/w/cpp/utility/functional/invoke
namespace detail
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//https://stackoverflow.com/questions/15411022/how-do-i-replace-a-tuple-element-at-compile-time
	/////////////////////////////////////////////////////////////////////////////////////////////////

	template <class T>
	concept is_intseq = requires (T a)
	{
		//Invoking this lambda to check the concept condition. 
		[] <class U, U... ints> (std::integer_sequence<U, ints...>) {}(a);
	};

	template <class F>
	concept is_void = requires()
	{
		std::is_same<std::invoke_result_t<F>, void>::value == true;
	};

	template <class F>
	concept is_not_void = requires()
	{
		std::is_same<std::invoke_result_t<F>, void>::value == false;
	};

	template <typename T>
	constexpr size_t paramLength(T)
	{
		return 1;
	}

	template <class T, T... ints>
	constexpr size_t paramLength(std::integer_sequence<T, ints...>)
	{
		return sizeof...(ints);
	}

	//Doesn't work for empty Args
	template <class T, class... Args>
	constexpr size_t nmbOfInvokes(T&& t, Args&&... args)
	{
		return (sizeof...(Args) == 0) ? paramLength(t) : 
			paramLength(t) * nmbOfInvokes<T, Args...>(t, args...);
	}

	template <class type_to_check, class Arg>
	constexpr bool compareTypes(Arg&& arg)
	{
		return std::is_same<type_to_check, decltype(arg)>::value;
	}

	template<class type_to_check, class... Args>
	constexpr bool wasThereAnyIntegerSequencePassed(Args&&... args)
	{
		return (... && compareTypes(std::forward<Args>(args)));
	}

	template <class F, class... Args>
	requires is_void<F>
	constexpr void invoke()
	{
		
	};

	// Function used to recursively perform all calls of F.
	template <class F, class... Args>
	requires is_not_void<F>
	constexpr void invoke(F&& f, Args&&... args)
	{
		constexpr size_t result_size = detail::nmbOfInvokes(args...);
		std::array<std::invoke_result_t<F>, result_size> invokeResults;

	};

	template <class F, class... Args, size_t SIZE>
	requires is_not_void<F>
		constexpr void invoke (F&& f, const std::array<std::invoke_result_t<F>, SIZE>, size_t index, Args&&... args)
	{
		
	};
} // namespace detail

template <class F, class... Args>
constexpr auto invoke_intseq(F&& f, Args&&... args) -> decltype(auto)
{// Delay the type deduction to the end of the function execution. 
	if constexpr (!(... || detail::is_intseq<decltype(args)>))
	{
		// There was no integer sequence passed, so we can just use std::invoke.
		return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
	}
	else
	{
		// TODO: recursive bullshit
	}
}

#endif // !INVOKE_INTSEQ_H
