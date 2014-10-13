/**
 * @file interpreter.hpp
 *
 * @brief
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */

#ifndef INTERPRETER_HPP_
#define INTERPRETER_HPP_ 1

#include <list>

#include "scanner.hpp"
#include "table.hpp"
#include "parser.hpp"


namespace tac {
	class Interpreter {
	public:
		Interpreter();

		~Interpreter();

		void run(const char *in, std::list<Error>&);

	private:
		Scanner *mp_scanner;
		SymbolTable *mp_table;
		Parser *mp_parser;

		bool compile(std::vector<Instruction*>*, std::list<Error>&, std::vector<Instruction>&);

		bool solve(Instruction*, std::list<Error>&);

		void execute(const std::vector<Instruction>&);
	};
}

#endif /* INTERPRETER_HPP_ */
