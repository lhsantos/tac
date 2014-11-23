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
 * @file scanner.hpp
 *
 * @brief The scanner (lexical analyzer) class definition.
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */

#ifndef SCANNER_HPP_
#define SCANNER_HPP_ 1

#include <string>
#include <list>

#include "parser.hpp"

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#undef  YY_DECL
#define YY_DECL \
	int tac::Scanner::yylex( \
			tac::Parser::semantic_type* yylval, \
			tac::Parser::location_type* yylloc)

namespace tac
{
	class Scanner: public yyFlexLexer
	{
	public:
		Scanner(std::istream *in);

		virtual ~Scanner();

		int yylex(
				Parser::semantic_type *yylval,
				Parser::location_type *yylloc);

	private:
		std::list<char> m_buffer;
	};
}

#endif /* SCANNER_HPP_ */
