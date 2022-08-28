#include "app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <utility>
int main()
{
	vmc::App app{};
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}