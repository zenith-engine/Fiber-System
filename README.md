# Fiber System

- Exemple 

```c++
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
	get_fiber_pool()->create_scripts();
	get_fiber_pool()->add([] {});
	while (true) {
		get_fiber_manager()->initialize();
		get_fiber_pool()->tick();
	}

	get_fiber_manager()->cleanup();
```
