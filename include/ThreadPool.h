#pragma once

#include <mutex>
#include <semaphore>
#include <vector>
#include <deque>
#include <thread>
#include <memory>

namespace RayTracer
{
	class ThreadPool
	{
	public:
		class IThreadPoolTask
		{
		public:
			virtual void Execute();
		protected:
			IThreadPoolTask() = default;
		private:
			IThreadPoolTask(IThreadPoolTask &) = delete;
			IThreadPoolTask(IThreadPoolTask &&) = delete;
		};

		ThreadPool(size_t thread_count, size_t queue_size);
		~ThreadPool();
		void EnqueueTask(const std::shared_ptr<IThreadPoolTask> &task);
		void Complete();
	private:
		ThreadPool() = delete;
		ThreadPool(ThreadPool &) = delete;
		ThreadPool(ThreadPool &&) = delete;
		void thread_loop();
		std::atomic<size_t> in_progress_task_count;

		std::mutex enqueued_tasks_lock;
		std::counting_semaphore<> queue_counter;

		std::deque<std::shared_ptr<IThreadPoolTask>> enqueued_tasks;
		std::vector<std::thread> threads;
		bool stop_requested;
	};
}