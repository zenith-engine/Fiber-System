#pragma once
#include "../stdafx.hpp"

namespace ve {
	class fiber {
	public:
		explicit fiber(std::string name, std::function<void()> func, std::optional<std::size_t> stackSize = std::nullopt, int priority = 0)
			: m_name(std::move(name)), m_func(std::move(func)), m_suspended(false), m_disabled(false), m_priority(priority),
			m_execution_time(0), m_interrupted(false), m_termination_timeout(std::nullopt) {

			std::size_t stack_size = stackSize.value_or(0);
			m_secondary = CreateFiber(stack_size, [](void* param) {
				static_cast<fiber*>(param)->run();
				}, this);
		}

		~fiber() {
			if (m_secondary) DeleteFiber(m_secondary);
		}

		void tick() {
			if (!m_disabled && !m_suspended) {
				m_primary = GetCurrentFiber();
				if (!m_time.has_value() || m_time.value() <= std::chrono::high_resolution_clock::now()) {
					SwitchToFiber(m_secondary);
				}
			}
		}

		void suspend() {
			m_suspended.store(true, std::memory_order_relaxed);
		}

		void resume() {
			m_suspended.store(false, std::memory_order_relaxed);
		}

		void terminate() {
			m_disabled.store(true, std::memory_order_relaxed);
		}

		bool is_suspended() const {
			return m_suspended.load(std::memory_order_acquire);
		}

		bool is_disabled() const {
			return m_disabled.load(std::memory_order_acquire);
		}

		const std::string& name() const {
			return m_name;
		}

		int priority() const {
			return m_priority;
		}

		long long execution_time() const {
			return m_execution_time;
		}

		bool is_interrupted() const {
			return m_interrupted.load(std::memory_order_acquire);
		}

		void set_termination_timeout(std::chrono::milliseconds timeout) {
			m_termination_timeout = timeout;
		}

	private:
		void run() {
			auto start = std::chrono::high_resolution_clock::now();
			try {
				m_func();
			}
			catch (const std::exception& ex) {
				std::cerr << "Exception in fiber '" << m_name << "': " << ex.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Unknown exception in fiber '" << m_name << "'" << std::endl;
			}
			auto end = std::chrono::high_resolution_clock::now();
			m_execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			while (!m_disabled.load(std::memory_order_acquire)) {
				sleep();
			}
		}
	public:
		void sleep(std::optional<std::chrono::high_resolution_clock::duration> time = std::nullopt) {
			if (time.has_value()) {
				m_time = std::chrono::high_resolution_clock::now() + time.value();
			}
			else {
				m_time = std::nullopt;
			}
			SwitchToFiber(m_primary);
		}

		static fiber* current() {
			return static_cast<fiber*>(GetFiberData());
		}

		void print_status() const {
			std::cout << "Fiber '" << m_name << "' "
				<< (m_disabled.load(std::memory_order_acquire) ? "Disabled" : "Active")
				<< ", Execution time: " << m_execution_time << "ms"
				<< ", Priority: " << m_priority
				<< ", Suspended: " << (m_suspended.load(std::memory_order_acquire) ? "Yes" : "No")
				<< std::endl;
		}

		void interrupt() {
			m_interrupted.store(true, std::memory_order_relaxed);
		}

		void set_state_callback(std::function<void(fiber*)> callback) {
			m_state_callback = callback;
		}

		void trigger_state_callback() {
			if (m_state_callback) {
				m_state_callback(this);
			}
		}

	public:
		std::string m_name;
		std::function<void()> m_func;
		std::atomic<bool> m_suspended;
		std::atomic<bool> m_disabled;
		std::atomic<bool> m_interrupted;
		void* m_primary{};
		void* m_secondary{};
		std::optional<std::chrono::high_resolution_clock::time_point> m_time;
		int m_priority;
		long long m_execution_time;
		std::optional<std::chrono::milliseconds> m_termination_timeout;
		std::function<void(fiber*)> m_state_callback;
	};
}