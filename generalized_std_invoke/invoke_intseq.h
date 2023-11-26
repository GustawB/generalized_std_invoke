#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <utility>
#include <array>

template<class... Args>
bool wasThereAnyIntegerSequencePassed(Args&&... args)
{
	// The idea is that the fold expression will take care of iterating
	// through the args list, and that the lambda will perform the type check
	// on every element (x). I hope that combining a lambda with templates
	// and std::is_same will produce a code that is evaluated
	// at the compilation time.
	return (... || [](auto&& x))
	{
		return (std::is_same(std::integer_sequence, decltype(x)));
	}
}

template <class F, class... Args>
constexpr auto invoke_intseq(F&& f, Args&&... args) -> decltype(auto)
{// Delay the type deduction to the end of the function execution. 
	if constexpr (!wasThereAnyIntegerSequencePassed(std::forward(args...)))
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
