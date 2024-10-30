#include "manager.hpp"

namespace ve {
	fiber_manager g_fiber_manager;

	void fiber_manager::initialize() {
		std::lock_guard lock(m_Mutex);
		if (!main_fiber_initialized) {
			if (!ConvertThreadToFiber(nullptr)) {
				throw std::runtime_error("Failed to convert main thread to fiber.");
			}
			main_fiber_initialized = true;
		}

		for (const auto& script : g_fibers) {
			if (!script->is_disabled()) {
				script->tick();
			}
		}
	}

	void fiber_manager::add(std::unique_ptr<fiber> script) {
		std::lock_guard lock(m_Mutex);
		if (!script) throw std::invalid_argument("Fiber script is null.");
		std::cout << std::format("Create: {}\n", script->name());
		g_fibers.push_back(std::move(script));
	}

	void fiber_manager::add(fiber* script) {
		std::lock_guard lock(m_Mutex);
		if (!script) {
			throw std::invalid_argument("Fiber script is null.");
		}
		std::cout << std::format("Create: {}", script->m_name) << std::endl;
		g_fibers.emplace_back(script);
	}

	void fiber_manager::add(const std::vector<std::pair<std::string, std::function<void()>>>& fibers) {
		std::lock_guard lock(m_Mutex);
		for (const auto& [name, func] : fibers) {
			auto script = std::make_unique<fiber>(name, func);
			std::cout << std::format("Create: {}", script->m_name) << std::endl;
			g_fibers.push_back(std::move(script));
		}
	}

	void fiber_manager::add(const std::string& name, std::function<void()> func) {
		std::lock_guard lock(m_Mutex);
		auto script = std::make_unique<fiber>(name, std::move(func));
		std::cout << "Create: " << script->m_name << std::endl;
		g_fibers.push_back(std::move(script));
	}

	void fiber_manager::cleanup() {
		std::lock_guard lock(m_Mutex);
		for (auto& fiber : g_fibers) {
			if (!fiber->is_disabled()) {
				fiber->terminate();
			}
		}
		g_fibers.clear();
	}

	void fiber_manager::suspend(const std::string& name) {
		std::lock_guard lock(m_Mutex);
		auto* target_fiber = find(name);
		if (target_fiber && !target_fiber->is_suspended()) {
			target_fiber->suspend();
			std::cout << std::format("Fiber suspended: {}\n", target_fiber->name());
		}
	}

	void fiber_manager::resume(const std::string& name) {
		std::lock_guard lock(m_Mutex);
		auto* target_fiber = find(name);
		if (target_fiber && target_fiber->is_suspended()) {
			target_fiber->resume();
			std::cout << std::format("Fiber resumed: {}\n", target_fiber->name());
		}
	}

	void fiber_manager::terminate(const std::string& name) {
		std::lock_guard lock(m_Mutex);
		auto* target_fiber = find(name);
		if (target_fiber && !target_fiber->is_disabled()) {
			target_fiber->terminate();
			std::cout << std::format("Fiber terminated: {}\n", target_fiber->name());
		}
	}

	void fiber_manager::list_active_fibers() const {
		std::lock_guard lock(m_Mutex);
		std::cout << "Listing all active fibers:\n";
		for (const auto& fiber : g_fibers) {
			if (!fiber->is_disabled()) {
				std::cout << std::format("Active fiber: {}\n", fiber->name());
			}
		}
	}

	fiber* fiber_manager::find(const std::string& name) {
		for (auto& fiber : g_fibers) {
			if (fiber->name() == name) {
				return fiber.get();
			}
		}
		return nullptr;
	}

	fiber_manager* get_fiber_manager() { return &g_fiber_manager; }
}