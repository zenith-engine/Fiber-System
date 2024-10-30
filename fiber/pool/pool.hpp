#pragma once
#include "../../stdafx.hpp"

namespace ve {
	class fiber_pool
	{
	public:
		void create_scripts();

		void tick();

		void add(std::function<void()> func);
	private:
		std::mutex m_mutex;
		std::stack<std::function<void()>> m_jobs;
	};

	fiber_pool* get_fiber_pool();
}