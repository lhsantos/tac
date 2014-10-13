/**
 * @file compiler.cpp
 *
 * @brief
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */

#include <fstream>

#include "interpreter.hpp"
#define ON_ERROR(msg, p) errors.push_back(Error(ERROR, msg, *p.filename, p.line, p.column))

namespace tac {
	Interpreter::Interpreter() : mp_scanner(0), mp_table(0), mp_parser(0) {}

	Interpreter::~Interpreter() {
		delete mp_scanner;
		delete mp_parser;
	}

	void Interpreter::run(const char *in, std::list<Error> &errors) {
		std::ifstream in_file(in);
		if (!in_file.good()) {
			errors.push_back(Error(ERROR, "couldn't open input file", in));
			return;
		}

		/* Parsers the input and generates a list of unsolved instructions. */
		std::vector<Instruction*> *unsolved = 0;

		delete (mp_scanner);
		mp_scanner = new Scanner(&in_file);

		delete (mp_table);
		mp_table = new SymbolTable();

		delete (mp_parser);
		mp_parser = new Parser(*mp_scanner, mp_table, unsolved, in, errors);

		std::cout << "parsing..." << std::endl;
		if (mp_parser->parse() == 2) {
			errors.push_back(Error(ERROR, "out of memory"));
			return;
		}
		/* If successful, tries to compile and run. */
		else {
			std::vector<Instruction> program;
			if (compile(unsolved, errors, program))
				execute(program);
		}
	}

	/* Compiles a list of unsolved instructions into a program (list of solved instructions). */
	bool Interpreter::compile(
			std::vector<Instruction*> *instructions,
			std::list<Error> &errors,
			std::vector<Instruction> &program) {

		std::cout << "compiling..." << std::endl;

		bool result = true;

		/* Tries to solve each instruction, storing an error flag. */
		program.clear();
		program.reserve(instructions->size());
		for (std::vector<Instruction*>::iterator i = instructions->begin(); i != instructions->end(); ++i) {
			if ((*i) && solve(*i, errors))
				program.push_back(**i);
			else
				result = false;

			delete (*i);
		}

		return result;
	}

	/* Given an instruction, tries to solve it. */
	bool Interpreter::solve(Instruction* instr, std::list<Error> &errors) {
		/* First, are there any pending symbols in the operands? */
		for (int i = 0; i < 3; ++i) {
			if (instr->operands[i].value.referee && (!instr->operands[i].solve(*mp_table))) {
				const Symbol *s = instr->operands[i].value.referee;
				ON_ERROR("unresolved symbol '" + *s->id + "'", s->loc.begin);
				return false;
			}
		}

		return true;
	}

	void Interpreter::execute(const std::vector<Instruction> &program) {
		std::cout << "running..." << std::endl;

		std::vector<Instruction>::const_iterator i;
		for (i = program.begin(); i != program.end(); ++i) {
			std::cout << i->to_str() << std::endl;
		}
	}
}
