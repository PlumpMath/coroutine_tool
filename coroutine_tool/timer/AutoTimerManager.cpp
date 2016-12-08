#include "AutoTimerManager.h"

//#include <xutility>



    AutoTimerManager::AutoTimerManager(boost::asio::io_service &io_service)
        :m_io_service(io_service)
	{

	}

	AutoTimerManager::~AutoTimerManager()
	{
		_clearAll();
	}

	auto_timer_holder_wp AutoTimerManager::registerTimer(boost::posix_time::time_duration time_delay, boost::posix_time::time_duration time_interval, int32 times, timer_callback &&func)
	{
		dk_weak_ptr<AutoTimerManager> this_wp = shared_from_this();
		auto_timer_type_up timer_up(new auto_timer_type(m_io_service));
		auto_timer_holder_sp holder_sp(new auto_timer_holder(timer_up, time_interval, time_delay, times, this_wp));
		holder_sp->setFunc(std::forward<timer_callback>(func));

		return _registerTimer_detail(holder_sp);
	}

	auto_timer_holder_wp AutoTimerManager::registerTimer(boost::posix_time::time_duration time_delay, boost::posix_time::time_duration time_interval, int32 times, timer_callback &func)
	{
		dk_weak_ptr<AutoTimerManager> this_wp = shared_from_this();
		auto_timer_type_up timer_up(new auto_timer_type(m_io_service));
		auto_timer_holder_sp holder_sp(new auto_timer_holder(timer_up, time_interval, time_delay, times, this_wp));
		holder_sp->setFunc(std::ref(func));

		return _registerTimer_detail(holder_sp);
	}

	IsFound AutoTimerManager::removeTimer(auto_timer_holder_wp &holder_wp)
	{
		if (holder_wp.expired())
			return IsFound::DESTROYED;

		auto sp = holder_wp.lock();
		return removeTimer(sp);
	}

	
	IsFound AutoTimerManager::removeTimer(auto_timer_holder_sp &holder_sp)
	{
		return _removeTimer(holder_sp);
	}

	void AutoTimerManager::clearAll()
	{
		_clearAll();
	}

	void AutoTimerManager::_clearAll()
	{
		_cancelAll();
		dk_unique_lock<dk_shared_mutex> locker(m_shared_mutex);
		m_timer_set.clear();
	}

	void AutoTimerManager::_cancelAll()
	{
		dk_shared_lock<dk_shared_mutex> locker(m_shared_mutex);
		for (auto it = m_timer_set.begin(); it != m_timer_set.end(); ++it)
		{
			auto &holder_sp = *it;
			holder_sp->m_timer_up->cancel();
		}
	}

	IsFound AutoTimerManager::_removeTimer(auto_timer_holder_sp &holder_sp)
	{
		dk_unique_lock<dk_shared_mutex> locker(m_shared_mutex);
		auto it = m_timer_set.find(holder_sp);
		if (it != m_timer_set.end())
		{
			auto &holder_sp = *it;
			holder_sp->m_timer_up->cancel();
			m_timer_set.erase(it);
			return IsFound::FOUND;
		}
		return IsFound::NOT_FOUND;
	}

	auto_timer_holder_wp AutoTimerManager::_registerTimer_detail(auto_timer_holder_sp holder_sp)
	{
		do {
			dk_unique_lock<dk_shared_mutex> locker(m_shared_mutex);
			m_timer_set.insert(holder_sp);
		} while (false);
		_beginTimer(holder_sp);
		return holder_sp;
	}

	void AutoTimerManager::_beginTimer(auto_timer_holder_sp &holder_sp)
	{
		_timerWrap(holder_sp, holder_sp->m_delay);
	}

	void AutoTimerManager::_timerWrap(auto_timer_holder_sp &holder_sp)
	{
		_timerWrap(holder_sp, holder_sp->m_interval);
	}

	void AutoTimerManager::_timerWrap(auto_timer_holder_sp &holder_sp, dk_time_duration &time_dutation)
	{
		dk_shared_lock<dk_shared_mutex> locker(m_shared_mutex);
		if (m_timer_set.find(holder_sp) == m_timer_set.end())
		{
			return;
		}
		auto &timer_up = holder_sp->m_timer_up;
		timer_up->expires_from_now(time_dutation);
		timer_up->async_wait(dk_bind(&auto_timer_holder::timerWraper, holder_sp, dk_1));
	}

	

	void auto_timer_holder::timerWraper(auto_timer_holder::THIS_SP holder_sp, const boost::system::error_code &err)
	{
		if (err == boost::asio::error::operation_aborted)
		{
			return;
		}
		auto mgr_sp = holder_sp->m_timer_manager_wp.lock();
		if (!mgr_sp)
		{
			//assert(false);
			return;
		}

		if (!holder_sp->m_func)
		{
			assert(false);
			return;
		}
		holder_sp->m_func(err, holder_sp);

		
		holder_sp->m_excuted_times += 1;
		if (holder_sp->m_times > 0 && holder_sp->m_excuted_times >= holder_sp->m_times)
		{
			mgr_sp->removeTimer(holder_sp);
		}
		else
		{
			mgr_sp->_timerWrap(holder_sp);
		}
	}

	auto_timer_holder::auto_timer_holder(auto_timer_type_up &timer_up, dk_time_duration interval, dk_time_duration delay, int32 times, dk_weak_ptr<AutoTimerManager> &timer_manager_wp) :m_timer_up(std::move(timer_up)), m_interval(interval), m_delay(delay),
		m_times(times), m_excuted_times(0), m_timer_manager_wp(timer_manager_wp)
	{
		//std::cout << "auto_timer_holder" << std::endl;
	}

	auto_timer_holder::~auto_timer_holder()
	{
		//std::cout << "~auto_timer_holder" << std::endl;
	}

	void auto_timer_holder::setFunc(const timer_callback &func)
	{
		m_func = func;
	}

	void auto_timer_holder::setFunc(const timer_callback &&func)
	{
		m_func = std::move(func);
	}

	void auto_timer_holder::cancel()
	{
		auto manager_sp = m_timer_manager_wp.lock();
		if (manager_sp)
		{
			manager_sp->removeTimer(shared_from_this());
		}
	}

	AutoTimerManager_strand::AutoTimerManager_strand(boost::asio::io_service::strand &strand)
		:AutoTimerManager(strand.get_io_service()), m_strand(strand)
	{

	}

	void AutoTimerManager_strand::_timerWrap(auto_timer_holder_sp &holder_sp, dk_time_duration &time_dutation)
	{
		dk_shared_lock<dk_shared_mutex> locker(m_shared_mutex);
		if (m_timer_set.find(holder_sp) == m_timer_set.end())
		{
			return;
		}
		auto &timer_up = holder_sp->m_timer_up;
		timer_up->expires_from_now(time_dutation);
		//timer_up->async_wait(m_strand.wrap(dk_bind(&auto_timer_holder::timerWraper, holder_sp, dk_1)));
		timer_up->async_wait(m_strand.wrap(dk_bind(&auto_timer_holder::timerWraper, holder_sp, dk_1)));
	}