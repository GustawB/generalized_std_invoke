#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <iostream>
#include <utility>
#include <array>

// Namespace containing all helper functions, naming based on the
// std::invoke documentation available here:
// https://en.cppreference.com/w/cpp/utility/functional/invoke
namespace detail
{
	template <class F>
	concept voidReturn = requires()
	{
		//Aparently doesn;t work, We need a type of every variable in the Args...
		std::is_same<std::invoke_result_t<F>, void>::value == true;
	};

	template <class type_to_check, class Arg>
	constexpr bool compareTypes(Arg&& arg)
	{
		return std::is_same<type_to_check, decltype(arg)>::value;
	}

	template<class type_to_check, class... Args>
	constexpr bool wasThereAnyIntegerSequencePassed(Args&&... args)
	{
		//return std::is_same<type_to_check, decltype(args)>::value;
		return (... && compareTypes(std::forward<Args>(args)));
	}

	// Function used to recursively perform all calls of F.
	template <class F, class... Args>
	constexpr auto invoke_all() -> decltype(auto)
	{

	}

	// invoke_all specialization for when the return type of F is void.
	template <class F, class... Args>
	requires voidReturn<F> // Concept that reuires this partial specialization to have a F function that returns void.
		constexpr auto invoke_all() -> decltype(auto)
	{

	}
} // namespace detail

template <class F, class... Args>
constexpr result_type invoke_intseq(F&& f, Args&&... args)
{// Delay the type deduction to the end of the function execution. 
	if constexpr (!detail::wasThereAnyIntegerSequencePassed<std::integer_sequence, Args>(std::forward<Args>(args...)))
	{
		std::cout << Args[i];
		// There was no integer sequence passed, so we can just use std::invoke.
		return std::invoke(std::forward<F>(f), std::forward(args)...);
	}
	else
	{
		// TODO: recursive bullshit
		std::invoke_result_t<F> result_type;
	}
}

#endif // !INVOKE_INTSEQ_H
