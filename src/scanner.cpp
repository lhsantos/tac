/**
 * @file scanner.cpp
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */

#include "scanner.hpp"
#include <cstdio>

namespace tac {
	Scanner::Scanner(std::istream *in) : yyFlexLexer(in, 0) { }

	Scanner::~Scanner() { }
}
