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
#include <iostream>
namespace concurrent {
	ThreadPool::ThreadPool(size_t num_threads):workers(num_threads),running(false),tasks() {}
	ThreadPool::ThreadPool():ThreadPool(4U) {}
	bool ThreadPool::is_running() const {
		return running;
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
		for(unsigned int i=0;i<workers.size();++i)
		{
			workers[i]=std::thread(&ThreadPool::do_task,this);
		}
	}
	void ThreadPool::stop() {
		running=false;
		for(size_t i=0;i<workers.size();++i)
		{
			workers[i].join();
		}
	}
	ThreadPool::~ThreadPool() {
		if(running) { stop(); }

	}
}

