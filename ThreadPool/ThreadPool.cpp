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
	void ThreadPool::task_loop() {
		while(running)
		{
			std::unique_ptr<ThreadTask> task;
			bool has_task;
			{
				std::lock_guard<std::mutex> guard(locker);
				if(has_task=!tasks.empty())
				{
					task=std::move(tasks.front());
					tasks.pop();
				}
			}
			if(has_task)
			{
				task->execute();
			}
			else
			{
				running=false;
			}
		}
	}
	void ThreadPool::start() {
		running=true;
		for(unsigned int i=0;i<workers.size();++i)
		{
			workers[i]=std::thread(&ThreadPool::task_loop,this);
		}
	}
	void ThreadPool::stop() {
		running=false;
		for(size_t i=0;i<workers.size();++i)
		{
			if(workers[i].joinable())
				workers[i].join();
		}
	}
	void ThreadPool::wait() {
		while(running);
		stop();
	}

	ThreadPool::~ThreadPool() {
		wait();
	}
}

