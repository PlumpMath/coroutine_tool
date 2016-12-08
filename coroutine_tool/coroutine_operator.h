#pragma once

#include <functional>

namespace coroutine_tool
{
	enum {
		ENUM_SLEEP_TIMER = 0x100,
	};

	enum {
		OP_SUCCESS = 0x0001,
		OP_FAILED_BECAUSE_UNKOWN,
		OP_AWAKE_BECAUSE_OUT_TIME,
		OP_AWAKE_BECAUSE_AWAKE_FUNC,
	};

	template<class yield_type>
	auto get_title(yield_type &yield)->decltype((std::get<0>(yield.get())))
	{
		return std::get<0>(yield.get());
	}

	template<class ENV, class yield_type>
	void passCondition(ENV &env, std::function<bool(yield_type &)> func)
	{
		while (true)
		{
			auto &yield = env.get_yield_ref();
			if (func(yield))
			{
				break;
			}

			yield();
		}
	}

	template<class ENV, class yield_type>
	int32 asyncSleep(ENV &env, dk_duration sleep_duration, std::function<bool(yield_type &)> awake_func = nullptr)
	{
		auto &yield = env.get_yield_ref();

		dk_shared_ptr<AutoTimerManager> timer_manager= env.getTimerManager();
		auto this_wp = env.weak_from_this();
		auto sleep_id = env.increaseSleepID();
		auto timer_wp = timer_manager->registerTimer(sleep_duration, dk_0_seconds, 1, [this_wp, sleep_id](TIME_CALLBACK_PARAMS_NONE){
			auto this_sp = this_wp.lock();
			if (this_sp)
			{
				env_title title(ENUM_SLEEP_TIMER, sleep_id, 0);
				this_sp->call_with_title(title);
			}
		});

		int32 result = OP_FAILED_BECAUSE_UNKOWN;
		std::function<bool(yield_type &)> func = [this_wp, sleep_id, &awake_func, timer_wp, &result](yield_type &yield)->bool{
			auto &title = get_title(yield);
			if (title.type == ENUM_SLEEP_TIMER)
			{
			 	if (title.param_low == sleep_id)
			 	{
			 		result = OP_AWAKE_BECAUSE_OUT_TIME;
			 		return true;
			 	}
			}

			if (awake_func)
			{
				if (awake_func(yield))
				{
					auto timer_sp = timer_wp.lock();
					if (timer_sp)
					{
						timer_sp->cancel();
					}

					result = OP_AWAKE_BECAUSE_AWAKE_FUNC;
					return true;
				}
			}

			return false;
		};
		passCondition(env, func);

		return result;
	}

	template<class ENV, class yield_type>
	void spawn(ENV &env, std::vector<std::function<void(yield_type &)>> &func_vec)
	{
		typedef ENV::wrapper_type wrapper_type;

		std::vector<wrapper_type> wrapper_vec;
		for (auto &f : func_vec)
		{
			wrapper_vec.push_back(wrapper_type(f));
		}

		while (true)
		{
			bool all_finished = true;

			auto &yield = env.get_yield_ref();

			for (auto &wrapper : wrapper_vec)
			{
				if (wrapper)
				{
					auto &yield_data = yield.get();

					all_finished = false;
					wrapper.call(yield_data);
				}
			}

			if (all_finished)
			{
				break;
			}

			yield();
		}
	}
}
