#pragma once

#include <tuple>
#include <memory>
#include <functional>
#include <atomic>

#include <boost/enable_shared_from_this.hpp>

#include "timer/AutoTimerManager.h"

#include "coroutine_operator.h"
#include "coroutine_wrapper.h"

namespace coroutine_tool{

	struct coroutine_exception
	{
		coroutine_exception(int _code, const std::string &_message)
			:code(_code), message(_message)
		{

		}

		virtual std::string tostring()
		{
			return message;
		}


		int code;
		std::string message;
	};

	template<class T>
	T &trans(T &t)
	{
		return t;
	}

	template<class T>
	T trans(T &&t)
	{
		return t;
	}

	struct env_title
	{
		env_title() :type(0), param64(0){}
		env_title(int32 _type) :type(_type), param64(0){}
		env_title(int32 _type, int32 low, int32 hight) :type(_type), param_low(low), param_hight(hight){}
		env_title(int32 _type, int32 _param64) :type(_type),param64(_param64){}

		int32 type;
		union {
			struct {
				int32 param_low;
				int32 param_hight;
			};
			int32 param64;
		};
	};

	template<class TYPE>
	class coroutine_env
	{

	};

	template<class ...ARGS>
	class coroutine_env<std::tuple<ARGS...>>
		: public boost::enable_shared_from_this < coroutine_env<std::tuple<ARGS...>> >
	{
	public:
		typedef coroutine_env<std::tuple<ARGS...>> MyT;
		typedef coroutine_wrapper<std::tuple<env_title, ARGS...>> wrapper_type;
		typedef typename wrapper_type::params_type params_type;
		typedef typename wrapper_type::cor_type::yield_type yield_type;

		typedef std::function<void(yield_type &)> spawn_func_type;
		typedef std::function<bool(yield_type &)> condition_func_type;

		//typedef typename wrapper_type::params_type tup_type;
	protected:
		coroutine_env()
			:m_wrapper(std::bind(&MyT::entrance, this, std::placeholders::_1)), m_timer_manager(nullptr), m_sleep_id(1000)
		{

		}
	public:
		void setIoService(boost::asio::io_service &io_service)
		{
			m_timer_manager.reset(new AutoTimerManager(io_service));
		}

		template<class ...TY>
		void call(env_title &title, TY &&... args)
		{
			m_wrapper.call(params_type(std::ref(title), trans(args)...));
		}

		void call_with_tuple(params_type &tup)
		{
			m_wrapper.call(tup);
		}

		void call_with_title(env_title &title)
		{
			char buff[sizeof(params_type)] = {0};
			params_type &params = *(reinterpret_cast<params_type *>(buff));
			std::get<0>(params) = title;
			call_with_tuple(params);
		}

		dk_shared_ptr<AutoTimerManager> getTimerManager()
		{
			return m_timer_manager;
		}

	protected:
		virtual void entrance(yield_type &yield)
		{
			return;
		}


	public:
		yield_type *get_yield_pointer()
		{
			return m_wrapper.get_yield_pointer();
		}

		yield_type &get_yield_ref()
		{
			auto *pointer = m_wrapper.get_yield_pointer();
			if (!pointer)
			{
				throw_err(-1, "yield_pointer is nullptr");
			}

			return *pointer;
		}

		virtual void throw_err(int code, const std::string &message)
		{
			throw_err(coroutine_exception(code, message));
		}

		void throw_err(coroutine_exception &e)
		{
			throw e;
		}

		struct scoped_timer
		{
			scoped_timer(auto_timer_holder_wp wp)
				:timer_wp(wp)
			{
				
			}

			scoped_timer(scoped_timer &&other)
				:timer_wp(other.timer_wp)
			{
				other.reset();
			}

			void release()
			{
				timer_wp.reset();
			}

			void cancel()
			{
				auto timer_sp = timer_wp.lock();
				if (timer_sp)
				{
					timer_sp->cancel();
				}

				timer_wp.reset();
			}

			~scoped_timer()
			{
				cancel();
			}
		private:
			auto_timer_holder_wp timer_wp;
		};

	public:
		int32 increaseSleepID()
		{
			return ++m_sleep_id;
		}

	protected:
		void passCondition(condition_func_type func)
		{
			coroutine_tool::passCondition(*this, func);
		}

		int32 asyncSleep(dk_duration sleep_duration, condition_func_type func = nullptr)
		{
			return coroutine_tool::asyncSleep(*this, sleep_duration, func);
		}

		void spawn(std::vector<spawn_func_type> &func_vec)
		{
			coroutine_tool::spawn(*this, func_vec);
		}

	private:
		wrapper_type m_wrapper;
		dk_shared_ptr<AutoTimerManager> m_timer_manager;

		std::atomic<int32> m_sleep_id;
	};

}