#include "stdafx.hpp"

using namespace ve;

int main() {
	get_fiber_manager()->add(std::make_unique<fiber>("Teste 1", [] {}));
	get_fiber_manager()->add(new fiber("Teste 2", [] {}));
	std::vector<std::pair<std::string, std::function<void()>>> fibers = {
		{
			"Teste 1", [] {
				std::cout << "Teste 1 Validated\n";
			}
		},
		{
			"Teste 2", [] {
				std::cout << "Teste 2 Validated\n";
			}
		},
		{
			"Teste 4", [] {
				std::cout << "Teste 4 Validated\n";
			}
		}
	};
	get_fiber_manager()->add(fibers);
	get_fiber_manager()->add("Teste 1", [] {});
	get_fiber_manager()->list_active_fibers();
	get_fiber_manager()->resume("Teste 2");
	get_fiber_manager()->suspend("Teste 2");
	get_fiber_manager()->terminate("Teste 2");
	get_fiber_manager()->find("Teste 2");
	get_fiber_pool()->initialize(5);
	get_fiber_pool()->add([] {}, 0);
	get_fiber_pool()->resize(7);
	get_fiber_pool()->set_max_jobs(5);
	auto stats = get_fiber_pool()->get_stats();
	std::cout << "=== Statistiques de Fiber Pool ===\n";
	std::cout << "Jobs en attente : " << stats.pending_jobs << "\n";
	std::cout << "Jobs executes : " << stats.executed_jobs << "\n";
	std::cout << "Fibres actives : " << stats.active_fibers << "\n";
	get_fiber_pool()->set_job_added_callback([](const ve::job& j) {
		std::cout << "[Callback] Job ajoute avec priorite " << j.priority << "\n";
		});
	get_fiber_pool()->set_job_executed_callback([](const ve::job& j) {
		std::cout << "[Callback] Job execute avec priorite " << j.priority << "\n";
		});
	get_fiber_pool()->set_job_rejected_callback([](const ve::job& j) {
		std::cerr << "[Callback] Job rejete : file d'attente pleine (priorite " << j.priority << ")\n";
		});

	auto job_func = [](int id) {
		std::cout << "Execution du job " << id << "\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		};

	for (int i = 0; i < 10; ++i) {
		bool success = get_fiber_pool()->add([=] { job_func(i); }, i % 3, std::chrono::milliseconds(i * 50));
		if (!success) {
			std::cerr << "[Main] Job " << i << " n'a pas pu etre ajoute.\n";
		}
	}

	get_fiber_manager()->resize(7);

	while (true) {
		get_fiber_manager()->initialize();
		get_fiber_pool()->tick();
	}
	get_fiber_manager()->cleanup();
	get_fiber_pool()->cleanup();
}