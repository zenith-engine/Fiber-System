#include "manager.hpp"

namespace ve {
    std::shared_ptr<fiber_manager> g_fiber_manager = std::make_shared<fiber_manager>();

	fiber_manager::fiber_manager()
		: m_main_fiber_initialized(false), m_active_fibers(0) {}

	void fiber_manager::initialize() {
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (!m_main_fiber_initialized) {
			if (!ConvertThreadToFiber(nullptr)) {
				throw std::runtime_error("Failed to convert main thread to fiber.");
			}
			m_main_fiber_initialized = true;
			std::cout << "[FiberManager] Main fiber initialized.";
		}

        for (const auto& script : m_fibers) {
            if (!script->is_disabled()) {
                script->tick();
            }
        }
	}

    void fiber_manager::resize(std::size_t new_size) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (new_size == m_fibers.size()) {
            std::cout << "[FiberManager] Taille inchange.";
            return;
        }

        if (new_size > m_fibers.size()) {
            std::size_t fibers_to_add = new_size - m_fibers.size();
            for (std::size_t i = 0; i < fibers_to_add; ++i) {
                auto fiber_name = std::format("Fiber_{}", m_fibers.size() + i);
                auto script = std::make_unique<fiber>(fiber_name, [] {
                    while (true) {
                        fiber::current()->sleep();
                    }
                    });
                std::cout << std::format("Ajout de la fibre : {}", fiber_name);
                m_fibers.push_back(std::move(script));
                ++m_active_fibers;
            }
        }
        else {
            std::size_t fibers_to_remove = m_fibers.size() - new_size;
            for (auto it = m_fibers.rbegin(); fibers_to_remove > 0 && it != m_fibers.rend(); ++it) {
                if ((*it)->is_disabled()) {
                    std::cout << std::format("Fibre déjà désactivée : {}\n", (*it)->name());
                }
                else {
                    (*it)->terminate();

                    if ((*it)->is_disabled()) {
                        std::cout << std::format("Fibre termine : {}\n", (*it)->name());
                        --m_active_fibers;
                        fibers_to_remove--;
                    }
                }
            }

            m_fibers.resize(new_size);
        }
        std::cout << std::format("Redimensionnement complet : taille actuelle = {}", m_fibers.size());
    }

	void fiber_manager::add(std::unique_ptr<fiber> script) {
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (!script) throw std::invalid_argument("Fiber script is null.");

		std::cout << std::format("Adding fiber: {}", script->name());
		m_fibers.push_back(std::move(script));
		++m_active_fibers;

		if (m_fiber_added_callback) {
			m_fiber_added_callback(m_fibers.back().get());
		}
	}

	void fiber_manager::add(fiber* script) {
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (!script) throw std::invalid_argument("Fiber script is null.");

		std::cout << std::format("Adding fiber: {}", script->m_name);
		m_fibers.emplace_back(script);
		++m_active_fibers;

		if (m_fiber_added_callback) {
			m_fiber_added_callback(script);
		}
	}

	void fiber_manager::add(const std::vector<std::pair<std::string, std::function<void()>>>& fibers) {
		std::lock_guard<std::mutex> lock(m_Mutex);

		for (const auto& [name, func] : fibers) {
			auto script = std::make_unique<fiber>(name, func);
			std::cout << std::format("Adding fiber: {}", script->name());
			m_fibers.push_back(std::move(script));
			++m_active_fibers;

			if (m_fiber_added_callback) {
				m_fiber_added_callback(m_fibers.back().get());
			}
		}
	}

	void fiber_manager::add(const std::string& name, std::function<void()> func) {
		std::lock_guard<std::mutex> lock(m_Mutex);

		auto script = std::make_unique<fiber>(name, std::move(func));
		std::cout << std::format("Adding fiber: {}", script->name());
		m_fibers.push_back(std::move(script));
		++m_active_fibers;

		if (m_fiber_added_callback) {
			m_fiber_added_callback(m_fibers.back().get());
		}
	}

	void fiber_manager::cleanup() {
		std::lock_guard<std::mutex> lock(m_Mutex);

		for (auto& fiber : m_fibers) {
			if (!fiber->is_disabled()) {
				fiber->terminate();
				std::cout << std::format("Terminated fiber: {}", fiber->name());
			}
		}

		m_fibers.clear();
		m_active_fibers = 0;

		if (m_cleanup_callback) {
			m_cleanup_callback();
		}
		std::cout << "[FiberManager] All fibers cleaned up.";
	}

    void fiber_manager::suspend(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto* target_fiber = find(name);
        if (target_fiber && !target_fiber->is_suspended()) {
            target_fiber->suspend();
            std::cout << std::format("Fiber suspended: {}", name);
            --m_active_fibers;

            if (m_fiber_suspended_callback) {
                m_fiber_suspended_callback(target_fiber);
            }
        }
    }

    void fiber_manager::resume(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto* target_fiber = find(name);
        if (target_fiber && target_fiber->is_suspended()) {
            target_fiber->resume();
            std::cout << std::format("Fiber resumed: {}", name);
            ++m_active_fibers;

            if (m_fiber_resumed_callback) {
                m_fiber_resumed_callback(target_fiber);
            }
        }
    }

    void fiber_manager::terminate(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto* target_fiber = find(name);
        if (target_fiber && !target_fiber->is_disabled()) {
            target_fiber->terminate();
            std::cout << std::format("Fiber terminated: {}", name);
            --m_active_fibers;

            if (m_fiber_terminated_callback) {
                m_fiber_terminated_callback(target_fiber);
            }
        }
    }

    void fiber_manager::list_active_fibers() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        std::cout << "Listing all active fibers:\n";
        for (const auto& fiber : m_fibers) {
            if (!fiber->is_disabled()) {
                std::cout << std::format("Active fiber: {}\n", fiber->name());
            }
        }
    }

    fiber* fiber_manager::find(const std::string& name) {
        for (auto& fiber : m_fibers) {
            if (fiber->name() == name) {
                return fiber.get();
            }
        }
        return nullptr;
    }

    void fiber_manager::set_fiber_added_callback(std::function<void(fiber*)> callback) {
        m_fiber_added_callback = std::move(callback);
    }

    void fiber_manager::set_fiber_suspended_callback(std::function<void(fiber*)> callback) {
        m_fiber_suspended_callback = std::move(callback);
    }

    void fiber_manager::set_fiber_resumed_callback(std::function<void(fiber*)> callback) {
        m_fiber_resumed_callback = std::move(callback);
    }

    void fiber_manager::set_fiber_terminated_callback(std::function<void(fiber*)> callback) {
        m_fiber_terminated_callback = std::move(callback);
    }

    void fiber_manager::set_cleanup_callback(std::function<void()> callback) {
        m_cleanup_callback = std::move(callback);
    }

    std::shared_ptr<fiber_manager> get_fiber_manager() { return g_fiber_manager; }
}