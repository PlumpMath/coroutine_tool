#pragma once

#include "coroutine_wrapper.h"
#include <tuple>
#include <memory>
#include <functional>


template<class TYPE>
class coroutine_env
{

};

template<class ...ARGS>
class coroutine_env<std::tuple<ARGS...>>
	: public std::enable_shared_from_this<coroutine_env<std::tuple<ARGS...>>>
{
public:
	typedef coroutine_env<std::tuple<ARGS...>> MyT;
	typedef coroutine_wrapper<std::tuple<ARGS...>> wrapper_type;
	typedef typename wrapper_type::params_type params_type;
	typedef typename wrapper_type::cor_type::yield_type yield_type;

	coroutine_env()
		:m_wrapper(std::bind(&MyT::entrance, this, std::placeholders::_1))
	{

	}

	template<class ...ARGS>
	void call(ARGS &&... args)
	{
		//m_wrapper.call(params_type((args)...));

		std::tuple<ARGS ...> tuppp;
		tuppp t((std::forward<ARGS>(args))...);
		
	}

protected:
	virtual void entrance(yield_type &yield)
	{
		return;
	}

private:
	wrapper_type m_wrapper;
};