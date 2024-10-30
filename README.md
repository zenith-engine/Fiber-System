# Fiber System
- A lightweight fiber system in C++ for creating, managing, and executing fibers. This system provides functionalities such as fiber creation, suspension, resuming, and termination.

# Table of Contents
- Features
- Installation
- Usage
- Code Examples
- API
- Contributing
- License
- 
# Features
- Create fibers with unique names.
- Add multiple fibers using vectors.
- Manage fiber states (suspended, resumed, terminated).
- Enumerate active fibers.
- Easy integration with lambda functions.

# Installation
To use this fiber system, simply include the provided header files in your C++ project and ensure your environment is set up to use Windows fiber functionalities (if required).

# Usage

- Here’s a sample of how to use the fiber system:

```c++
#include <iostream>
#include "fiber.hpp"
#include "manager.hpp"
#include "queue.hpp"

int main() {
    // Adding fibers with lambdas
    get_fiber_manager()->add(std::make_unique<fiber>("Test 1", [] {
        std::cout << "Test 1 Executed\n";
    }));

    get_fiber_manager()->add(new fiber("Test 2", [] {
        std::cout << "Test 2 Executed\n";
    }));

    // Adding multiple fibers via a vector
    std::vector<std::pair<std::string, std::function<void()>>> fibers = {
        {"Test 1", [] { std::cout << "Test 1 Validated\n"; }},
        {"Test 2", [] { std::cout << "Test 2 Validated\n"; }},
        {"Test 4", [] { std::cout << "Test 4 Validated\n"; }},
    };
    get_fiber_manager()->add(fibers);

    // Adding a fiber with a lambda function
    get_fiber_manager()->add("Test 1", [] {});

    // Listing active fibers
    get_fiber_manager()->list_active_fibers();

    // Suspending, resuming, and terminating a fiber
    get_fiber_manager()->resume("Test 2");
    get_fiber_manager()->suspend("Test 2");
    get_fiber_manager()->terminate("Test 2");

    // Finding a fiber
    get_fiber_manager()->find("Test 2");

    // Managing the fiber pool
    get_fiber_pool()->create_scripts();
    get_fiber_pool()->add([] { std::cout << "Executing pooled fiber\n"; });

    // Main loop to execute fibers
    while (true) {
        get_fiber_manager()->initialize();
        get_fiber_pool()->tick();
    }

    // Cleaning up fibers
    get_fiber_manager()->cleanup();
    return 0;
}
```

# Features Breakdown
- Fiber Creation: Add fibers using `add()`, passing a name and a function.
- Fiber Management: Use `suspend()`, `resume()`, and `terminate()` to manage fiber states.
- Active Fiber Enumeration: Use `list_active_fibers()` to view currently active fibers.
- Integration with a Fiber Pool: Create and manage a pool of fibers for efficient task handling.

# API
`get_fiber_manager()`
Returns the global fiber manager, allowing access to all fiber management methods.

`add(std::unique_ptr<fiber>)`
Adds a unique fiber to the manager.

`add(new fiber(name, func))`
Adds a new fiber using the new operator.

`add(const std::vector<std::pair<std::string, std::function<void()>>>& fibers)`
Adds multiple fibers in a single operation.

`add(const std::string& name, std::function<void()> func)`
Adds a fiber by specifying its name and function.

`suspend(const std::string& name)`
Suspends the specified fiber by name.

`resume(const std::string& name)`
Resumes the execution of the specified fiber.

`terminate(const std::string& name)`
Terminates the specified fiber.

`find(const std::string& name)`
Returns information about the specified fiber.

`list_active_fibers()`
Displays all active fibers.

# Contributing
- Contributions are welcome! Please submit a pull request or open an issue to discuss improvements.
