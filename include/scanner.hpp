/**
 * @file scanner.hpp
 *
 * @brief The scanner (lexical analyzer) class definition.
 *
 * @date 10/10/2014
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

namespace tac {
	class Scanner: public yyFlexLexer {
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
