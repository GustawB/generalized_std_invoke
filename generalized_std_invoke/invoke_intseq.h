#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <iostream>
#include <functional>
#include <utility>
#include <array>
#include <tuple>

// Namespace containing all helper functions, naming based on the
// std::invoke documentation available here:
// https://en.cppreference.com/w/cpp/utility/functional/invoke
namespace detail
{
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

	// Function used to recursively perform all calls of F.
	template <class F, class... Args>
	struct invoke
	{
		static constexpr void run()
		{
			//I guess there are no more params, only function, so we can invoke it. 
		}
	};
} // namespace detail

template <class F, class... Args>
constexpr auto invoke_intseq(F&& f, Args&&... args) -> decltype(auto)
{// Delay the type deduction to the end of the function execution. 
	if constexpr (!detail::wasThereAnyIntegerSequencePassed<std::integer_sequence, Args...>(std::forward<Args...>(args...)))
	{
		// There was no integer sequence passed, so we can just use std::invoke.
		return std::invoke(std::forward<F>(f), std::forward(args)...);
	}
	else
	{
		std::tuple<Args...> argsTuple;
		for (size_t i = 0; i < std::tuple_size<decltype(argsTuple)>::value; ++i)
		{
			if constexpr (std::is_same<decltype(std::get<i>(argsTuple)), std::integer_sequence>)
			{

			}
		}
		// TODO: recursive bullshit
	}
}

#endif // !INVOKE_INTSEQ_H
