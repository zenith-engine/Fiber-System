#pragma once
#include "../stdafx.hpp"

namespace ve {
	class fiber {
	public:
		fiber(std::string name, std::function<void()> func, std::optional<std::size_t> stackSize = std::nullopt)
			: m_name(std::move(name)), m_func(std::move(func)), m_suspended(false), m_disabled(false)
		{
			m_secondary = CreateFiber(stackSize.value_or(0), [](void* param) {
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

		void suspend() { m_suspended = true; }
		void resume() { m_suspended = false; }
		void terminate() { m_disabled = true; }

		bool is_suspended() const { return m_suspended; }
		bool is_disabled() const { return m_disabled; }

		const std::string& name() const { return m_name; }

	private:
		void run() {
			m_func();
			while (!m_disabled) {
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

	public:
		std::string m_name;
		std::function<void()> m_func;
		bool m_suspended;
		bool m_disabled;
		void* m_primary{};
		void* m_secondary{};
		std::optional<std::chrono::high_resolution_clock::time_point> m_time{};
	};
}