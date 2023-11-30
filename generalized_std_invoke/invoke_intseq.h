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
	// Wrapper to hold a type of a 'simple variable.
	template <typename T>
	struct Wrapper 
	{
		using type = T;
	};

	// Wrapper to swap intseq to intconst
	template <typename T, T... ints>
	struct Wrapper<std::integer_sequence < T, ints...>>
	{
		using type = typename std::integral_constant<T, 0>; // 0 is a dummy variable.
	};

	// Concept to check whether we have a intseq in Args... or not.
	template <class T>
	concept is_intseq = requires (T a)
	{
		//Invoking this lambda to check the concept condition. 
		[] <class U, U... ints> (std::integer_sequence<U, ints...>) {}(a);
	};

	// Wrapper wrapping the size of a "simple" variable (always 1).
	template <class T>
	struct sizeCalc
	{
		static constexpr size_t size = 1;
	};

	// Wrapper wrapping the size of a intseq.
	template <class T, T... ints>
	struct sizeCalc<std::integer_sequence<T, ints...>>
	{
		static constexpr size_t size = sizeof...(ints);
	};

	template <typename T>
	constexpr static size_t paramLength(T&&)
	{
		return 1;
	}

	template <class T, T... ints>
	constexpr static size_t paramLength(std::integer_sequence<T, ints...>&&)
	{
		return sizeof...(ints);
	}

	//Doesn't work for empty Args
	template <class T, class... Args>
	constexpr static size_t nmbOfInvokes(T&& t, Args&&... args)
	{
		if constexpr (sizeof...(Args) == 0)
		{
			return paramLength(std::forward<T>(t));
		}
		else
		{
			return paramLength(std::forward<T>(t)) *
				nmbOfInvokes<T, Args...>(std::forward<T>(t), std::forward<Args>(args)...);
		}
	}

	template <class type_to_check, class Arg>
	constexpr bool compareTypes(Arg&& arg)
	{
		return std::is_same_v<type_to_check, decltype(arg)>;
	}

	template<class type_to_check, class... Args>
	constexpr bool wasThereAnyIntegerSequencePassed(Args&&... args)
	{
		return (... && compareTypes(std::forward<Args>(args)));
	}

	// Main logic function, overload for F's that return void.
	template <class F, class... Args>
	requires std::is_void_v<std::invoke_result_t<F, typename Wrapper<Args>::type...>>
		constexpr void invoke(F&& f, Args&&... args)
	{

	};

	// Main logic function, overload for F's that return non-void.
	template <class F, class... Args>
		constexpr auto invoke(F&& f, Args&&... args) -> decltype(auto)
	{
		constexpr size_t result_size = (... * sizeCalc<Args>::size);
		std::array<std::invoke_result_t<F, 
			typename Wrapper<Args>::type...>, result_size> results;

		return results;
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
		return detail::invoke(std::forward<F>(f), std::forward<Args>(args)...);
	}
}

#endif // !INVOKE_INTSEQ_H