#pragma once

#include <memory>
#include <functional>
#include <stdint.h>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define dk_shared_ptr boost::shared_ptr
#define dk_unique_ptr std::unique_ptr
#define dk_weak_ptr boost::weak_ptr
#define dk_enable_shared_from_this boost::enable_shared_from_this
#define dk_unorder_set boost::unordered_set

#define dk_mutex boost::mutex
#define dk_shared_mutex boost::shared_mutex
#define dk_unique_lock boost::lock_guard
#define dk_shared_lock boost::shared_lock_guard

#define dk_function std::function
#define dk_bind std::bind
#define dk_1 std::placeholders::_1
#define dk_2 std::placeholders::_2
#define dk_3 std::placeholders::_3
#define dk_4 std::placeholders::_4
#define dk_5 std::placeholders::_5

typedef int32_t int32;
typedef int64_t int64;

#define TIME_CALLBACK_PARAMS(p1, p2) \
	const boost::system::error_code &p1, dk_shared_ptr<auto_timer_holder> &p2

#define TIME_CALLBACK_PARAMS_NONE TIME_CALLBACK_PARAMS(_ignore_param_hahahahagagaga,_ignore_param_hohohohohahaha)

#define TIME_CALLBACK_PARAMS_ERROR(err) TIME_CALLBACK_PARAMS(err,_ignore_param_hohohohohahaha)

#define TIME_CALLBACK_PARAMS_TIMER(timer_ptr) TIME_CALLBACK_PARAMS(_ignore_param_hahahahagagaga, timer_ptr)




class LogicSession;
class AutoTimerManager;

typedef boost::asio::deadline_timer auto_timer_type;

typedef dk_unique_ptr<auto_timer_type> auto_timer_type_up;

typedef boost::system::error_code timer_callback_error_code;

typedef boost::posix_time::seconds dk_seconds;
typedef boost::posix_time::milliseconds dk_milliseconds;
typedef boost::posix_time::microseconds dk_microseconds;
typedef boost::posix_time::time_duration dk_time_duration;

typedef dk_seconds dk_s;
typedef dk_milliseconds dk_ms;
typedef dk_microseconds dk_us;
typedef boost::posix_time::time_duration dk_duration;
static const dk_seconds dk_0_seconds(0);

struct auto_timer_holder :public boost::enable_shared_from_this<auto_timer_holder>
{
	typedef dk_shared_ptr<auto_timer_holder> THIS_SP;
	typedef dk_weak_ptr<auto_timer_holder> THIS_WP;

	typedef dk_function<void(const boost::system::error_code &, dk_shared_ptr<auto_timer_holder> &)> timer_callback;

	auto_timer_type_up m_timer_up;
	dk_time_duration m_interval;
	dk_time_duration m_delay;
	int32 m_times;
	timer_callback m_func;

	~auto_timer_holder();

private:
	int32 m_excuted_times;
	dk_weak_ptr<AutoTimerManager> m_timer_manager_wp;

	//void _add_param_wraper(const boost::system::error_code &, THIS_SP &);

public:
	static void timerWraper(THIS_SP holder_sp, const boost::system::error_code &err);

	auto_timer_holder(auto_timer_type_up &timer_up, dk_time_duration interval, 
		dk_time_duration delay, int32 times, dk_weak_ptr<AutoTimerManager> &timer_manager_wp);

		
	void setFunc(const timer_callback &func);
	void setFunc(const timer_callback &&func);

	void cancel();
		
};

typedef auto_timer_holder::THIS_SP auto_timer_holder_sp;
typedef auto_timer_holder::THIS_WP auto_timer_holder_wp;

typedef auto_timer_holder::timer_callback timer_callback;



enum class IsFound{FOUND,NOT_FOUND,DESTROYED};

class AutoTimerManager:public dk_enable_shared_from_this<AutoTimerManager>
{
public:
	AutoTimerManager(boost::asio::io_service &io_service);

	virtual ~AutoTimerManager();
		
	auto_timer_holder_wp registerTimer(
		boost::posix_time::time_duration time_delay,
		boost::posix_time::time_duration time_interval,
		int32 times,
		timer_callback &&func
		);

	auto_timer_holder_wp registerTimer(
		boost::posix_time::time_duration time_delay,
		boost::posix_time::time_duration time_interval,
		int32 times,
		timer_callback &func
		);
		

	IsFound removeTimer(auto_timer_holder_wp &holder_wp);
	IsFound removeTimer(auto_timer_holder_sp &holder_sp);
	void clearAll();

protected:
	virtual void _clearAll();
	virtual void _cancelAll();
	virtual IsFound _removeTimer(auto_timer_holder_sp &holder_sp);
	virtual auto_timer_holder_wp _registerTimer_detail(auto_timer_holder_sp holder_sp);

	virtual void _beginTimer(auto_timer_holder_sp &holder_sp);
	void _timerWrap(auto_timer_holder_sp &holder_sp);
	virtual void _timerWrap(auto_timer_holder_sp &holder_sp, dk_time_duration &time_dutation);
		
	friend auto_timer_holder;


protected:
	dk_unorder_set<auto_timer_holder_sp> m_timer_set;
		
	boost::asio::io_service &m_io_service;

	dk_shared_mutex m_shared_mutex;// 仅使用于对m_timer_set的操作
};

class AutoTimerManager_strand :public AutoTimerManager
{
public:
	AutoTimerManager_strand(boost::asio::io_service::strand &strand);

	virtual void _timerWrap(auto_timer_holder_sp &holder_sp, dk_time_duration &time_dutation) override;

protected:
	boost::asio::strand &m_strand;
};


