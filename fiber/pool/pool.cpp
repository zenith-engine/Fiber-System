#include "pool.hpp"

namespace ve {
	std::shared_ptr<fiber_pool> g_fiber_pool = std::make_shared<fiber_pool>();

	fiber_pool::fiber_pool(std::uint32_t max_jobs)
		: m_max_jobs(max_jobs) {}

	void fiber_pool::initialize(std::uint32_t pool_size) {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto manager = get_fiber_manager();

		if (!manager) {
			throw std::runtime_error("Fiber manager not initialized.");
		}

		m_running = true;
		m_executed_jobs = 0;

		for (std::uint32_t i = 0; i < pool_size; ++i) {
			std::string fiber_name = "FiberPool_" + std::to_string(i);
			manager->add(std::make_unique<fiber>(fiber_name, [this] {
				while (m_running) {
					this->tick();
					fiber::current()->sleep();
				}
				}));
		}

		std::cout << "[FiberPool] Initialized with " << pool_size << " fibers.\n";
	}

	void fiber_pool::tick() {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_cv.wait(lock, [this] { return !m_jobs.empty() || !m_running; });

		if (!m_running) return;

		auto now = std::chrono::steady_clock::now();
		if (!m_jobs.empty() && m_jobs.top().ready_time <= now) {
			auto job = std::move(m_jobs.top());
			m_jobs.pop();

			if (job.expiration_time <= now) {
				std::cout << "[FiberPool] Skipped expired job with priority " + std::to_string(job.priority);
				return;
			}

			lock.unlock();

			try {
				std::invoke(std::move(job.func));
				++m_executed_jobs;
				if (m_job_executed_callback) {
					m_job_executed_callback(job);
				}
				std::cout << "[FiberPool] Executed job with priority " + std::to_string(job.priority);
			}
			catch (const std::exception& e) {
				std::cout << std::string("[FiberPool] Job execution error: ") + e.what();
			}
		}
		else {
			if (!m_jobs.empty()) {
				m_cv.wait_until(lock, m_jobs.top().ready_time);
			}
		}
	}

	void fiber_pool::cleanup() {
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_running = false;
		}
		m_cv.notify_all();
		std::cout << "[FiberPool] Shutting down...\n";
	}

	void fiber_pool::resize(std::uint32_t new_size) {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto manager = get_fiber_manager();

		if (!manager) {
			throw std::runtime_error("Fiber manager not initialized.");
		}

		std::uint32_t current_size = m_fibers.size();
		if (new_size > current_size) {
			for (std::uint32_t i = current_size; i < new_size; ++i) {
				std::string fiber_name = "FiberPool_" + std::to_string(i);
				manager->add(std::make_unique<fiber>(fiber_name, [this] {
					while (m_running) {
						this->tick();
						fiber::current()->sleep();
					}
					}));
			}
			std::cout << "[FiberPool] Resized to " << new_size << " fibers.\n";
		}
		else if (new_size < current_size) {
			for (std::uint32_t i = new_size; i < current_size; ++i) {
				m_fibers[i]->terminate();
			}
			m_fibers.resize(new_size);
			std::cout << "[FiberPool] Reduced to " << new_size << " fibers.\n";
		}
	}

	bool fiber_pool::add(std::function<void()> func, int priority, std::chrono::steady_clock::duration delay, std::chrono::steady_clock::duration expiration) {
		if (func) {
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_jobs.size() >= m_max_jobs) {
				if (m_job_rejected_callback) {
					job rejected_job{ std::move(func), std::chrono::steady_clock::now() + delay, priority, std::chrono::steady_clock::now() + expiration };
					m_job_rejected_callback(rejected_job);
				}
				std::cout << "[FiberPool] Job rejected: queue full.";
				return false;
			}

			job new_job{
				std::move(func),
				std::chrono::steady_clock::now() + delay,
				priority,
				std::chrono::steady_clock::now() + expiration
			};
			m_jobs.push(std::move(new_job));

			if (m_job_added_callback) {
				m_job_added_callback(new_job);
			}

			std::cout << "[FiberPool] Added job with priority " + std::to_string(priority) + ".";
			m_cv.notify_one();
			return true;
		}
		return false;
	}

	fiber_pool::stats fiber_pool::get_stats() {
		std::lock_guard<std::mutex> lock(m_mutex);
		return { m_jobs.size(), m_executed_jobs, m_fibers.size() };
	}

	std::size_t fiber_pool::get_fiber_count() {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_fibers.size();
	}

	void fiber_pool::set_max_jobs(std::size_t max_jobs) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_max_jobs = max_jobs;
	}

	void fiber_pool::set_verbosity(bool verbose) {
		m_verbose = verbose;
	}

	void fiber_pool::set_job_added_callback(std::function<void(const job&)> callback) {
		m_job_added_callback = std::move(callback);
	}

	void fiber_pool::set_job_executed_callback(std::function<void(const job&)> callback) {
		m_job_executed_callback = std::move(callback);
	}

	void fiber_pool::set_job_rejected_callback(std::function<void(const job&)> callback) {
		m_job_rejected_callback = std::move(callback);
	}

	std::shared_ptr<fiber_pool> get_fiber_pool() { return g_fiber_pool; }
}