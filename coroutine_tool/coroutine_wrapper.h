#pragma once

#include <boost/coroutine/all.hpp>
#include <mutex>
#include <tuple>

template<class PARAM_TUPLE_TYPE>
class coroutine_wrapper
{

};

template<class ...ARGS>
class coroutine_wrapper<std::tuple<ARGS...>>
{
public:
	typedef std::tuple<ARGS &&...> params_type;

	typedef coroutine_wrapper<std::tuple<ARGS...>> _MyT;
	typedef boost::coroutines::symmetric_coroutine<std::tuple<ARGS...>> cor_type;
	typedef std::function<void(typename cor_type::yield_type &)> reg_func_type;

	inline coroutine_wrapper(reg_func_type func)
		:m_fn(func), m_call(m_fn)
	{
	
	}

	inline bool operator !() const
	{
		return !m_call;
	}

	inline operator bool() const
	{
		return m_call.operator bool();
	}

	
	inline void call(params_type &param)
	{
		std::lock_guard<std::recursive_mutex> locker(m_mutex);
		if (m_call)
		{
			m_call(param);
		}
	}

private:
	std::recursive_mutex m_mutex;
	reg_func_type m_fn;
	typename cor_type::call_type m_call;
};