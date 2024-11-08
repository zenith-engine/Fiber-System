#pragma once
#include "../../stdafx.hpp"

namespace ve {
	struct job {
		std::function<void()> func;
		std::chrono::steady_clock::time_point ready_time;
		int priority;
		std::chrono::steady_clock::time_point expiration_time;

		bool operator<(const job& other) const {
			if (priority == other.priority) {
				return ready_time > other.ready_time;
			}
			return priority < other.priority;
		}
	};
	class fiber_pool : public std::enable_shared_from_this<fiber_pool>
	{
	public:
		struct stats {
			std::size_t pending_jobs;
			std::size_t executed_jobs;
			std::size_t active_fibers;
		};

		explicit fiber_pool(std::uint32_t max_jobs = 1000, bool verbose = true);

		void initialize(std::uint32_t pool_size);

		void resize(std::uint32_t new_size);

		bool add(std::function<void()> func, int priority, std::chrono::steady_clock::duration delay = std::chrono::milliseconds(0), std::chrono::steady_clock::duration expiration = std::chrono::minutes(5));

		void set_job_added_callback(std::function<void(const job&)> callback);
		void set_job_executed_callback(std::function<void(const job&)> callback);
		void set_job_rejected_callback(std::function<void(const job&)> callback);

		void tick();

		void cleanup();

		stats get_stats();

		std::size_t get_fiber_count();

		void set_max_jobs(std::size_t max_jobs);
		void set_verbosity(bool verbose);
	private:
		mutable std::mutex m_mutex;
		std::condition_variable m_cv;
		std::priority_queue<job> m_jobs;
		bool m_running = false;
		std::atomic<std::size_t> m_max_jobs{ 1000 };
		bool m_verbose{ true };
		std::vector<std::unique_ptr<fiber>> m_fibers;
		std::atomic<std::size_t> m_executed_jobs{ 0 };
		std::function<void(const job&)> m_job_added_callback;
		std::function<void(const job&)> m_job_executed_callback;
		std::function<void(const job&)> m_job_rejected_callback;
	};

	std::shared_ptr<fiber_pool> get_fiber_pool();
}