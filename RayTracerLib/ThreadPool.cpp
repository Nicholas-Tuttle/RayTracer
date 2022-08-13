#include "ThreadPool.h"

using RayTracer::ThreadPool;

void ThreadPool::IThreadPoolTask::Execute()
{
	return;
}

void ThreadPool::thread_loop()
{
	while (!stop_requested)
	{
		std::shared_ptr<ThreadPool::IThreadPoolTask> task = nullptr;
		
		{
			const std::lock_guard<std::mutex> lock(enqueued_tasks_lock);
			if (!enqueued_tasks.empty())
			{
				{
					task = enqueued_tasks.front();
					enqueued_tasks.pop_front();
				}
			}
		}

		if(nullptr != task)
		{
			in_progress_task_count++;
			queue_counter.release();
			task->Execute();
			in_progress_task_count--;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}
}

ThreadPool::ThreadPool(size_t thread_count, size_t queue_size)
	: stop_requested(false), in_progress_task_count(0), enqueued_tasks_lock(), queue_counter(queue_size), enqueued_tasks()
{
	size_t actual_threads = thread_count < 1 ? 1 : thread_count;
	for (int index = 0; index < actual_threads; index++)
	{
		threads.emplace_back(std::thread(&ThreadPool::thread_loop, this));
	}
}

ThreadPool::~ThreadPool()
{
	stop_requested = true;

	for(auto & thread: threads)
	{
		thread.join();
	}

	threads.clear();
}

void ThreadPool::EnqueueTask(const std::shared_ptr<IThreadPoolTask> &task)
{
	queue_counter.acquire();
	const std::lock_guard<std::mutex> lock(enqueued_tasks_lock);
	enqueued_tasks.push_back(task);
}

void ThreadPool::Complete()
{
	bool tasks_enqueued = true;
	while (tasks_enqueued)
	{
		const std::lock_guard<std::mutex> lock(enqueued_tasks_lock);
		tasks_enqueued = !enqueued_tasks.empty() || (in_progress_task_count != 0);

		if (tasks_enqueued)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}
}
