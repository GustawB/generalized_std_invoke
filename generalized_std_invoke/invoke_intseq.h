#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

template<class F, class... ArgTypes>
class result_type;

template<class ...Args>
bool isThereAnyIntegerSequencePassed

template <class F, class... Args>
constexpr result_type<class F, Args...>
	invoke_intseq(F&& f, Args&&... args);

#endif // !INVOKE_INTSEQ_H
