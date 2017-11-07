/*
Copyright(C) 2017 Edward Xie

This program is free software:you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation,either version 3 of the License,or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not,see <https://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "ThreadPool.h"
namespace concurrent {
	ThreadPool::ThreadPool(size_t num_threads):num_workers(num_threads),workers(0),running(false),tasks() {}
	ThreadPool::ThreadPool():ThreadPool(1U) {}
	std::unique_ptr<ThreadTask> ThreadPool::pop_task() {
		std::lock_guard<std::mutex> guard(locker);
		std::unique_ptr<ThreadTask> do_task=std::move(tasks.front());
		tasks.pop();
		return do_task;
	}
	bool ThreadPool::is_running() const {
		return running;
	}
	bool ThreadPool::has_task() {
		std::lock_guard<std::mutex> guard(locker);
		return !tasks.empty();
	}
	void ThreadPool::do_task() {
		std::unique_ptr<ThreadTask> do_task;
		while(running)
		{
			bool has_task;
			{
				std::lock_guard<std::mutex> guard(locker);
				if(has_task=!tasks.empty())
				{
					do_task=std::move(tasks.front());
					tasks.pop();
				}
			}
			if(has_task)
			{
				do_task->execute();
			}
		}
	}
	void ThreadPool::start() {
		running=true;
		for(unsigned int i=0;i<num_workers;++i)
		{
			workers.emplace_back(&ThreadPool::do_task,this);
		}
	}
	void ThreadPool::stop() {
		running=false;
		for(size_t i=0;i<num_workers;++i)
		{
			workers[i].join();
		}
	}
	ThreadPool::~ThreadPool() {
		stop();
	}
}
