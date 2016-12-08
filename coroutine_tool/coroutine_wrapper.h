#pragma once

#include <mutex>
#include <tuple>
#include <boost/coroutine/all.hpp>

namespace coroutine_tool{

	template<class PARAM_TUPLE_TYPE>
	class coroutine_wrapper
	{

	};

	template<class ...ARGS>
	class coroutine_wrapper < std::tuple<ARGS...> >
	{
	public:
		typedef std::tuple<ARGS ...> params_type;

		typedef coroutine_wrapper<std::tuple<ARGS...>> _MyT;
		typedef boost::coroutines::symmetric_coroutine<std::tuple<ARGS...>> cor_type;
		typedef std::function<void(typename cor_type::yield_type &)> reg_func_type;
		typedef typename cor_type::yield_type yield_type;

		inline coroutine_wrapper(reg_func_type func)
			:m_fn(func), m_call(std::bind(&_MyT::call_wrapper, this, std::placeholders::_1)), m_yield_pointer(nullptr)
		{

		}

		inline coroutine_wrapper(coroutine_wrapper &&other)
			: m_fn(other.m_fn), m_call(std::bind(&_MyT::call_wrapper, this, std::placeholders::_1)), m_yield_pointer(nullptr)
		{
			other.m_fn = nullptr;
		}

		inline bool operator !() const
		{
			return !m_call;
		}

		inline operator bool() const
		{
			return m_call.operator bool();
		}


		void call(params_type &param)
		{
			std::lock_guard<std::recursive_mutex> locker(m_mutex);
			if (m_call)
			{
				m_call(param);
			}
		}

		yield_type *get_yield_pointer()
		{
			return m_yield_pointer;
		}

		const yield_type *get_yield_pointer() const
		{
			return m_yield_pointer;
		}
	private:
		struct on_exit
		{
			on_exit(_MyT &wrapper, yield_type &yield)
				:m_wrapper(wrapper)
			{
				wrapper.m_yield_pointer = &yield;
			}
			~on_exit()
			{
				m_wrapper.m_yield_pointer = nullptr;
			}

		private:
			_MyT &m_wrapper;
		};

		void call_wrapper(yield_type &yield)
		{
			on_exit _on_exit(*this, yield);

			if (m_fn)
			{
				m_fn(yield);
			}
		}

	private:
		std::recursive_mutex m_mutex;
		reg_func_type m_fn;
		typename cor_type::call_type m_call;
		yield_type *m_yield_pointer;
	};

}