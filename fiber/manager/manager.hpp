#pragma once
#include "../../stdafx.hpp"

namespace ve {
	class fiber_manager : public std::enable_shared_from_this<fiber_manager> {
	public:
		explicit fiber_manager();

		void initialize();
		void add(std::unique_ptr<fiber> script);
		void add(fiber* script);
		void add(const std::vector<std::pair<std::string, std::function<void()>>>& fibers);
		void add(const std::string& name, std::function<void()> func);
		void cleanup();

		void suspend(const std::string& name);
		void resume(const std::string& name);
		void terminate(const std::string& name);
		void list_active_fibers();

		void resize(std::size_t new_size);

		void set_fiber_added_callback(std::function<void(fiber*)> callback);
		void set_fiber_suspended_callback(std::function<void(fiber*)> callback);
		void set_fiber_resumed_callback(std::function<void(fiber*)> callback);
		void set_fiber_terminated_callback(std::function<void(fiber*)> callback);
		void set_cleanup_callback(std::function<void()> callback);

		void set_verbosity(bool verbose);

	public:
		fiber* find(const std::string& name);

	private:
		bool m_main_fiber_initialized;
		std::size_t m_active_fibers;
		std::vector<std::unique_ptr<fiber>> m_fibers;
		std::mutex m_Mutex;

		std::function<void(fiber*)> m_fiber_added_callback;
		std::function<void(fiber*)> m_fiber_suspended_callback;
		std::function<void(fiber*)> m_fiber_resumed_callback;
		std::function<void(fiber*)> m_fiber_terminated_callback;
		std::function<void()> m_cleanup_callback;
	};

	std::shared_ptr<fiber_manager> get_fiber_manager();
}
