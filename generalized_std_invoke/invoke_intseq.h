#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <type_traits>
#include <utility>

template<class F, class... ArgTypes>
class result_type;

template<class ...Args>
bool isThereAnyIntegerSequencePassed(Args... args)
{
	//The idea is that the fold expression will take care of iterating
	//through the args list, and that the lambda will perform the type check
	//on every element (x). I hope that combining a lambda with templates
	//andstd::is_same will produce a code that is evaluated
	//at the compilation time.
	return (... || [](auto x))
	{
		return (std::is_same(std::integer_sequence, decltype(x)));
	}
}

template <class F, class... Args>
constexpr result_type<class F, Args...>
	invoke_intseq(F&& f, Args&&... args);

#endif // !INVOKE_INTSEQ_H
