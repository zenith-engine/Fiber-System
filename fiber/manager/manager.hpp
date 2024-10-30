#pragma once
#include "../../stdafx.hpp"

namespace ve {
	class fiber_manager {
	public:
		void initialize();
		void add(std::unique_ptr<fiber> script);
		void add(fiber* script);
		void add(const std::vector<std::pair<std::string, std::function<void()>>>& fibers);
		void add(const std::string& name, std::function<void()> func);
		void cleanup();

		void suspend(const std::string& name);
		void resume(const std::string& name);
		void terminate(const std::string& name);
		void list_active_fibers() const;

	public:
		fiber* find(const std::string& name);

	private:
		bool main_fiber_initialized{ false };
		mutable std::mutex m_Mutex;
		std::vector<std::unique_ptr<fiber>> g_fibers;
	};

	fiber_manager* get_fiber_manager();
}
