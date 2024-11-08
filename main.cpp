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
	auto pool = get_fiber_pool();
	pool->set_max_jobs(5);
	pool->set_verbosity(true);
	auto stats = pool->get_stats();
	std::cout << "=== Statistiques de Fiber Pool ===\n";
	std::cout << "Jobs en attente : " << stats.pending_jobs << "\n";
	std::cout << "Jobs exécutés : " << stats.executed_jobs << "\n";
	std::cout << "Fibres actives : " << stats.active_fibers << "\n";
	pool->set_job_added_callback([](const ve::job& j) {
		std::cout << "[Callback] Job ajouté avec priorité " << j.priority << "\n";
		});
	pool->set_job_executed_callback([](const ve::job& j) {
		std::cout << "[Callback] Job exécuté avec priorité " << j.priority << "\n";
		});
	pool->set_job_rejected_callback([](const ve::job& j) {
		std::cerr << "[Callback] Job rejeté : file d'attente pleine (priorité " << j.priority << ")\n";
		});

	auto job_func = [](int id) {
		std::cout << "Exécution du job " << id << "\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		};

	for (int i = 0; i < 10; ++i) {
		bool success = pool->add([=] { job_func(i); }, i % 3, std::chrono::milliseconds(i * 50));
		if (!success) {
			std::cerr << "[Main] Job " << i << " n'a pas pu être ajouté.\n";
		}
	}

	while (true) {
		get_fiber_manager()->initialize();
		get_fiber_pool()->tick();
	}
	get_fiber_manager()->cleanup();
	get_fiber_pool()->cleanup();
}