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
 * @file scanner.cpp
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */

#include "scanner.hpp"
#include <cstdio>

namespace tac {
	Scanner::Scanner(std::istream *in) : yyFlexLexer(in, 0) { }

	Scanner::~Scanner() { }
}
