/*
Copyright 2014 Luciano Henrique de Oliveira Santos

This file is part of TAC project.

TAC project is licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file main.cpp
 *
 * @brief Program entry point.
 *
 * @date 2014-10-11
 *
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
        { "verbose",        no_argument, 0, 'v' },
        { "brief",          no_argument, 0, 'b' },
        { "debug",          no_argument, 0, 'd' },
        { "step",           no_argument, 0, 's' },
        { "show-labels",    no_argument, 0, 'l' },
        { 0, 0, 0, 0 }
    };

    std::list<Error> errors;
    int errcount = 0;
    uint8_t opts = 0;

    int c;
    while ((c = getopt_long(argc, argv, "vbdsl", long_options, 0)) != -1)
    {
        switch (c)
        {
        case 0: break;
        case 'v': opts |= Interpreter::VERBOSE; break;
        case 'b': opts &= (~Interpreter::VERBOSE); break;
        case 'd': opts |= Interpreter::DEBUG; break;
        case 's': opts |= Interpreter::STEP; break;
        case 'l': opts |= Interpreter::DLABELS; break;
        case '?':
        default:
            std::cerr << "option '" << (char) optopt << "' is invalid: ignored" << std::endl;
            break;
        }
    }

    Interpreter i(opts);
    i.run(argv[optind], errors);
    if (errors.size() > 0)
    {
        for (auto e : errors)
        {
            e.print(std::cerr);
            std::cerr << std::endl;

            if (e.errorLevel() == ERROR)
                ++errcount;
        }
    }

    return errcount ? -1 : 0;
}
