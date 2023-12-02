#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <iostream>
#include <functional>
#include <utility>
#include <vector>
#include <array>
#include <tuple>

//https://en.cppreference.com/w/cpp/utility/functional/bind_front

// Namespace containing all helper functions, naming based on the
// std::invoke documentation available here:
// https://en.cppreference.com/w/cpp/utility/functional/invoke
namespace detail
{
	template <typename T>
	struct is_integer_sequence : std::false_type {};

	template <typename T, T... Ints>
	struct is_integer_sequence<std::integer_sequence<T, Ints...>> : std::true_type {};

	template <typename T>
	constexpr bool is_integer_sequence_v = is_integer_sequence<T>::value;

	template<class T>
	using result_type = T;

	// Wrapper to hold a type of a 'simple' variable.
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

	template<typename F, typename Args>
	constexpr auto bind_front(F&& f, Args&& args)
	{
		return[f = std::forward<F>(f), args = std::forward<Args>(args)](auto&&... moreArgs) -> decltype(auto)
		{
			return f(args, moreArgs...);
		};
	}

	template<class F>
	constexpr void void_invoke(F&& f) 
	{
		f(); // There were no more args, so we just invoke the function.
	}

	// Main logic function, overload for F's that return void.
	template <typename F, typename T, typename... Args>
		constexpr void void_invoke(F&& f, T&& arg, Args&&... args)
	{
		void_invoke(bind_front(std::forward<F>(f), std::forward<T>(arg)), std::forward<Args>(args)...);
	};

	template <typename F, typename... Args, typename T, T... ints>
		constexpr void void_invoke(F&& f, std::integer_sequence<T, ints...>&&, Args&&... args) 
	{
		(void_invoke(bind_front(std::forward<F>(f), std::integral_constant<T, ints>()), std::forward<Args>(args)...), ...);
	};

	template<class F>
	constexpr decltype(auto) get_first_result(F&& f)
	{
		return f();
	}

	template <class F, class T, class... Args>
	constexpr decltype(auto) get_first_result(F&& f, T&& arg, Args&&... args)
	{
		return get_first_result(bind_front(std::forward<F>(f), std::forward<T>(arg)), std::forward<Args>(args)...);
	}

	template <class F, class... Args, class T, T... ints>
	constexpr decltype(auto) get_first_result(F&& f, std::integer_sequence<T, ints...>, Args&&... args)
	{
		get_first_result(bind_front(std::forward<F>(f), std::integral_constant<T, 0>()), std::forward<Args>(args)...);
	}

	template<class F, class return_type>
	constexpr decltype(auto) non_void_invoke(F&& f, return_type&&)
	{
		std::vector result{ f() };
		return result;
	}

	template <class F, class return_type, class T, class... Args>
	constexpr decltype(auto) non_void_invoke(F&& f, return_type&& result_type, T&& arg, Args&&... args)
	{
		return non_void_invoke(bind_front(std::forward<F>(f), std::forward<T>(arg)), std::forward<return_type>(result_type), std::forward<Args>(args)...);
	}

	template <class F, class return_type, class... Args, class T, T... ints>
	constexpr decltype(auto) non_void_invoke(F&& f, return_type&& result_type, std::integer_sequence<T, ints...>, Args&&... args)
	{
		std::vector<return_type> results;
		([&]
			{
				for (const auto& result : non_void_invoke(bind_front(std::forward<F>(f), std::integral_constant<T, ints>()),
					std::forward<return_type>(result_type), std::forward<Args>(args)...))
				{
					results.emplace_back(result);
				}
			}(), ...);

		return results;
	}
} // namespace detail

template <class F, class... Args>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args)
{// Delay the type deduction to the end of the function execution. 
	if constexpr ((... || detail::is_integer_sequence_v<std::decay_t<Args>>))
	{
		using return_type = std::invoke_result_t<F, typename detail::Wrapper<Args>::type...>;
		// TODO: recursive bullshit
		if constexpr (std::is_void_v<return_type>)
		{// Void function.
			detail::void_invoke(std::forward<F>(f), std::forward<Args>(args)...);
		}
		else
		{

			return_type result_type;
			return detail::non_void_invoke(std::forward<F>(f), std::forward<return_type>(result_type), std::forward<Args>(args)...);
		}
	}
	else
	{
		// There was no integer sequence passed, so we can just use std::invoke.
		return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
	}
}

#endif // !INVOKE_INTSEQ_H