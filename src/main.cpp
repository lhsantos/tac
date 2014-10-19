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

#include "getopt.h"

#include "interpreter.hpp"

using namespace tac;

int main(const int argc, char **argv)
{
	static struct option long_options[] =
	{
		{ "verbose", no_argument, 0, 'v' },
		{ "brief",	 no_argument, 0, 'b' },
		{ "debug",	 no_argument, 0, 'd' },
		{ "step",	 no_argument, 0, 's' },
		{ 0, 0, 0, 0 }
	};

	std::list<Error> errors;
	int actualErrors = 0;
	uint8_t opts = 0;

	int c;
	while ((c = getopt_long(argc, argv, "vbds", long_options, 0)) != -1)
	{
		switch (c) {
		case 0: break;
		case 'v': opts |= Interpreter::VERBOSE; break;
		case 'b': opts &= (~Interpreter::VERBOSE); break;
		case 'd': opts |= Interpreter::DEBUG; break;
		case 's': opts |= Interpreter::STEP; break;
		case '?':
		default:
			std::cerr << "option '" << (char) optopt << "' is invalid: ignored" << std::endl;
			break;
		}
	}

	Interpreter i(opts);
	i.run(argv[optind], errors);
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
