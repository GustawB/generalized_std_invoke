#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <iostream>
#include <functional>
#include <utility>
#include <vector>
#include <array>
#include <tuple>

// Namespace containing all helper functions, naming based on the
// std::invoke documentation available here:
// https://en.cppreference.com/w/cpp/utility/functional/invoke
namespace detail
{
	// Three structs used to determine whether the given param pack
	// contains any intseq.
	template <typename T>
	struct is_integer_sequence : std::false_type {};

	template <typename T, T... Ints>
	struct is_integer_sequence<std::integer_sequence<T, Ints...>> :
		std::true_type {};

	template <typename T>
	constexpr bool is_integer_sequence_v = is_integer_sequence<T>::value;

	// Wrapper to hold a type of a 'simple' variable. for return type check.
	template <typename T>
	struct Wrapper
	{
		using type = T;
	};

	// Wrapper to swap intseq to intconst for return type check.
	template <typename T, T... ints>
	struct Wrapper<std::integer_sequence < T, ints...>>
	{
		using type = typename std::integral_constant < T, (T) {} > ;
	};

	// Function used to implement a behaviour similar to cuttying.
	// Thanks to that, we can do stuff like F(arg1)(rest) -> 
	// F(arg1, arg2)(rest) etc, and the call the F when it was binded
	// with all its args. I use it instead of std::bind_front (except the
	// recursive_invoke) because it produced errors I couldn't fix (vice versa)
	// for recursive_invoke.
	template<typename F, typename Args>
	constexpr auto bind_front(F&& f, Args&& args) -> decltype(auto)
	{
		return[f = std::forward<F>(f), args = std::forward<Args>(args)]
		(auto&&... moreArgs) -> decltype(auto)
		{
			return f(args, moreArgs...);
		};
	}

	// Overload for when there are no more aruments.
	template<class F>
	constexpr void void_invoke(F&& f)
	{
		f();
	}

	// Overload fo when the current argument is not an intseq.
	template <typename F, typename T, typename... Args>
	constexpr void void_invoke(F&& f, T&& arg, Args&&... args)
	{
		void_invoke(bind_front(std::forward<F>(f), std::forward<T>(arg)),
			std::forward<Args>(args)...);
	};

	// Overload for when the current argument is an intseq. Because this 
	// function is called when f returns void, all we need to do is to
	// invoke it for all param combinations, so we don't need to 
	// iterate over results of lover recursive calls,
	// just over all possible params.
	template <typename F, typename... Args, typename T, T... ints>
	constexpr void void_invoke(F&& f, std::integer_sequence<T, ints...>&&,
		Args&&... args)
	{
		(..., void_invoke(bind_front(std::forward<F>(f),
			std::integral_constant<T, ints>()), std::forward<Args>(args)...));
	};

	// Overload for when there are no more aruments.
	template<class F>
	constexpr auto get_first_result(F&& f) -> decltype(auto)
	{
		return f();
	}

	// Overload fo when the current argument is not an intseq.
	template <class F, class T, class... Args>
	constexpr auto get_first_result(F&& f, T&& arg, 
		Args&&... args) -> decltype(auto)
	{
		return get_first_result(std::bind_front(std::forward<F>(f), 
			std::forward<T>(arg)), std::forward<Args>(args)...);
	}

	// Overload for when the current argument is an intseq. This function needs
	// to return only one value, so I'm just swapping intseq with intconst
	// of the respective type.
	template <class F, class... Args, class T, T... ints>
	constexpr auto get_first_result(F&& f, std::integer_sequence<T, ints...>, 
		Args&&... args) -> decltype(auto)
	{
		return get_first_result(std::bind_front(std::forward<F>(f), 
			std::integral_constant < T, (T) {} > ()),
			std::forward<Args>(args)...);
	}

	// Overload for when there are no more aruments.
	template<class F, class return_type>
	constexpr decltype(auto) non_void_invoke(F&& f, return_type&&)
	{
		// We will iterate over results, on higher levels of the recursion,
		// so we need a wrapper (vector) that we can iterate over. 
		std::vector<return_type> result{ f() };
		return result;
	}

	// Overload fo when the current argument is not an intseq.
	template <class F, class return_type, class T, class... Args>
	constexpr decltype(auto) non_void_invoke(F&& f, return_type&& result_type,
		T&& arg, Args&&... args)
	{
		return non_void_invoke(bind_front(std::forward<F>(f), 
			std::forward<T>(arg)), std::forward<return_type>(result_type),
			std::forward<Args>(args)...);
	}

	// Overload for when the current argument is an intseq and we need to 
	// iterate over all its values.
	template <class F, class return_type, class... Args, class T, T... ints>
	constexpr decltype(auto) non_void_invoke(F&& f, return_type&& result_type,
		std::integer_sequence<T, ints...>, Args&&... args)
	{
		std::vector<return_type> results;
		(..., [&]
			{
				for (const auto& result : non_void_invoke(bind_front(
					std::forward<F>(f), std::integral_constant<T, ints>()),
					std::forward<return_type>(result_type), 
					std::forward<Args>(args)...))
				{// reference_invoke will return a vector of some type, so 
					// I'm just iterating oover all results of all lover
					// recursive cals, and then I'm adding them to 
					// results vector.
					results.emplace_back(result);
				}
			}());

		return results;
	}

	// Overload for when there are no more params.
	template<class F, class return_type>
	constexpr decltype(auto) reference_invoke(F&& f, const return_type&)
	{
		// We will iterate over results, on higher levels of the recursion,
		// so we need a wrapper (vector) that we can iterate over.
		std::vector<return_type> result{ f() };
		return result;
	}

	// Overload fo when the current argument is not an intseq.
	template <class F, class return_type, class T, class... Args>
	constexpr auto reference_invoke(F&& f, const return_type& type, T&& arg, 
		Args&&... args) -> decltype(auto)
	{
		return reference_invoke(std::bind_front(std::forward<F>(f),
			std::forward<return_type>(arg)), type, std::forward<Args>(args)...);
	}

	// Overload for when the current argument is an intseq and we need to 
	// iterate over all its values.
	template <class F, class return_type, class... Args, class T, T... ints>
	constexpr auto reference_invoke(F&& f, const return_type& type,
		std::integer_sequence<T, ints...>, Args&&... args) -> decltype(auto)
	{
		std::vector<return_type> results; 
		(..., [&]
			{
				for (const auto& result : reference_invoke(std::bind_front(
					std::forward<F>(f), std::integral_constant<T, ints>()),
					type, std::forward<Args>(args)...))
				{// reference_invoke will return a vector of some type, so 
					// I'm just iterating oover all results of all lover
					// recursive cals, and then I'm adding them to 
					// results vector.
					results.emplace_back(result);
				}
			}());
		return results;
	}
} // namespace detail

template <class F, class... Args>
constexpr auto invoke_intseq(F&& f, Args&&... args) -> decltype(auto)
{// Delay the type deduction to the end of the function execution. 
	if constexpr ((... || detail::is_integer_sequence_v<std::decay_t<Args>>))
	{// Case when there was at least one intseq passed as the function param.
		using return_type = 
			std::invoke_result_t<F, typename detail::Wrapper<Args>::type...>;
		if constexpr (std::is_void_v<return_type>)
		{// f function returns void.
			detail::void_invoke(std::forward<F>(f), 
				std::forward<Args>(args)...);
		}
		else
		{// f returns something non-void.
			if constexpr (std::is_reference_v<return_type>)
			{// f returns a reference.
				// Every value returned by f will be of the same type, so 
				// here I'm extracting the first result, so later I can pass
				// it to the next levels of recursion and use its type to 
				// declare std::vector holding results (I can't just simply 
				// do "return_type result_type" because for references I would
				// need an ninitializer).
				auto type = detail::get_first_result(
					std::forward<F>(f), std::forward<Args>(args)...);
				return detail::reference_invoke(std::forward<F>(f), 
					type, std::forward<Args>(args)...);
			}
			else
			{// f returns a value.
				return_type type; // Placeholder value used to 
				// somehow pass the f's function return type to the next 
				// levesl of recursion.
				return detail::non_void_invoke(std::forward<F>(f), 
					std::forward<return_type>(type),
					std::forward<Args>(args)...);
			}
		}
	}
	else
	{// There was no integer sequence passed, so we can just use std::invoke.
		return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
	}
}

#endif // !INVOKE_INTSEQ_H