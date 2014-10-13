/**
 * @file main.cpp
 *
 * @brief Program entry point.
 *
 * @date 11/10/2014
 * @author Luciano Santos
 */

#include <iostream>
#include <cstdlib>
#include <list>

#include "error.hpp"
#include "interpreter.hpp"

using namespace tac;

void process(const int argc, const char **argv, std::list<Error>& errors) {
	Interpreter i;

	if (argc != 2) {
		errors.push_back(Error(ERROR, "no input file provided"));
		return;
	}

	i.run(argv[1], errors);
}

int main(const int argc, const char **argv) {
	std::list<Error> errors;
	int actualErrors = 0;

	process(argc, argv, errors);

	if (errors.size() > 0) {
		for (std::list<Error>::iterator i = errors.begin(); i != errors.end(); ++i) {
			i->print(std::cerr);
			std::cerr << std::endl;

			if (i->errorLevel() == ERROR)
				++actualErrors;
		}
	}

	return actualErrors ? -1 : 0;
}
