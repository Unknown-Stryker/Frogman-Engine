﻿/*
Copyright © from 2022 to present, UNKNOWN STRYKER. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <FE/framework/task.hpp>
#include <FE/fqueue.hxx>

// std::mutex, std::lock_guard
#include <mutex>

#include <boost/thread.hpp>




BEGIN_NAMESPACE(FE::framework)


namespace internal::thread_id
{
	static FE::fqueue<FE::aligned<var::uint8, FE::align_CPU_L1_cache_line>, 255> s_thread_ids;
	static std::uint8_t s_next_thread_id = 0;
	static std::mutex s_lock;

	class __generator
	{
		var::uint8 m_thread_id;

	public:
		__generator() noexcept
		{
			std::lock_guard<std::mutex> l_lock(s_lock);
			if (s_thread_ids.is_empty() == false)
			{
				this->m_thread_id = s_thread_ids.pop()._data;
				return;
			}
			FE_ASSERT(s_next_thread_id < 255);
			this->m_thread_id = s_next_thread_id;
			++s_next_thread_id;
		}

		~__generator() noexcept
		{
			std::lock_guard<std::mutex> l_lock(s_lock);
			s_thread_ids.push((FE::aligned<var::uint8, FE::align_CPU_L1_cache_line>)this->m_thread_id);
		}

		FE::uint8 get_id() const noexcept
		{
			return this->m_thread_id;
		}
	};
}

FE::uint8 get_current_thread_id() noexcept
{
	thread_local static internal::thread_id::__generator tl_s_id_generator;
	return tl_s_id_generator.get_id();
}




latent_event::latent_event(const function_type& task_p, FE::float64 delay_p) noexcept : m_event(task_p), m_delay_in_milliseconds(delay_p), m_timer()
{
	this->m_timer.start_clock();
}

latent_event::latent_event(const latent_event& other_p) noexcept : m_event(other_p.m_event), m_delay_in_milliseconds(other_p.m_delay_in_milliseconds), m_timer(other_p.m_timer)
{
}

latent_event::latent_event(latent_event&& other_p) noexcept : m_event(std::move(other_p.m_event)), m_delay_in_milliseconds(other_p.m_delay_in_milliseconds), m_timer(std::move(other_p.m_timer))
{
}

latent_event& latent_event::operator=(const latent_event& other_p) noexcept
{
	this->m_event = other_p.m_event;
	this->m_delay_in_milliseconds = other_p.m_delay_in_milliseconds;
	this->m_timer = other_p.m_timer;
	return *this;
}

latent_event& latent_event::operator=(latent_event&& other_p) noexcept
{
	this->m_event = std::move(other_p.m_event);
	this->m_delay_in_milliseconds = other_p.m_delay_in_milliseconds;
	this->m_timer = std::move(other_p.m_timer);
	return *this;
}

FE::boolean latent_event::is_ready() noexcept
{
	this->m_timer.end_clock();
	return this->m_timer.get_delta_time() >= this->m_delay_in_milliseconds;
}

void latent_event::operator()()
{
	this->m_event();
}




task_scheduler::task_scheduler(uint32 max_concurrency_p) noexcept 
	: m_is_inturrupted(false), m_executor(max_concurrency_p)
{
}

task_scheduler::~task_scheduler() noexcept
{
	this->interrupt();
}

tf::Executor& task_scheduler::access_executor() noexcept
{
	return this->m_executor;
}

void task_scheduler::interrupt() noexcept
{
	this->m_is_inturrupted.store(true, std::memory_order_release);
	this->m_executor.wait_for_all();
}

void task_scheduler::launch_latent_event(const typename latent_event::function_type& task_p, FE::uint64 delay_in_milliseconds_p) noexcept
{
	boost::thread l_launcher
	(
		[task_p, delay_in_milliseconds_p, this]()
		{
			tf::Taskflow l_taskflow;
			l_taskflow.emplace( [task_p](){ task_p(); } );
			boost::this_thread::sleep_for(boost::chrono::milliseconds(delay_in_milliseconds_p));
			this->access_executor().run(l_taskflow).wait();
		}
	);

	l_launcher.detach();
}


END_NAMESPACE


