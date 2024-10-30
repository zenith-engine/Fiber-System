#include "pool.hpp"

namespace ve {
	fiber_pool g_fiber_pool;

	void fiber_pool::create_scripts() {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto* manager = get_fiber_manager();

		if (!manager) {
			throw std::runtime_error("Fiber manager not initialized.");
		}

		for (int i = 0; i < 8; ++i) {
			std::string fiber_name = "FiberPool_" + std::to_string(i);

			manager->add(std::make_unique<fiber>(fiber_name, [this] {
				while (true) {
					this->tick();
					fiber::current()->sleep();
				}
				}));
		}
	}

	void fiber_pool::tick() {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (!m_jobs.empty()) {
			auto job = std::move(m_jobs.top());
			m_jobs.pop();
			std::invoke(std::move(job));
		}
	}

	void fiber_pool::add(std::function<void()> func) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_jobs.push(std::move(func));
	}

	fiber_pool* get_fiber_pool() { return &g_fiber_pool; }
}