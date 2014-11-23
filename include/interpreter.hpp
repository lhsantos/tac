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
 * @file interpreter.hpp
 *
 * @brief
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */

#ifndef INTERPRETER_HPP_
#define INTERPRETER_HPP_ 1

#include <list>
#include <vector>
#include <stdint.h>

#include "error.hpp"
#include "scanner.hpp"
#include "table.hpp"
#include "memmngr.hpp"
#include "parser.hpp"
#include "instruction.hpp"


namespace tac
{
	class TACExecutionException : public TACException
	{
	public:
		TACExecutionException(const position&, const std::string&) throw();

		virtual ~TACExecutionException() throw();

		virtual const std::string msg() const;

	private:
		const std::string m_msg;
	};


	class Interpreter
	{
	public:
		enum Options
		{
			VERBOSE = 1,
			DEBUG = 2,
			STEP = 4
		};

		static const uint STACK_REG_CODE;
		static const uint FRAME_REG_CODE;
		static const uint PC_REG_CODE;
		static const uint RA_REG_CODE;


		Interpreter(uint8_t);

		~Interpreter();

		void run(const char *in, std::list<Error>&);

	private:
		class Context
		{
		public:
			Context(Interpreter *interpreter);

			~Context();

			Context* new_child(const location &loc, uint return_address, uint param_count);

			Context* parent() const;

			Symbol* get_param(uint, const location&) const;

			Symbol* get_temp(uint, const location&);

			Symbol* get(uint, const location&) const;

			uint get_param_addr(uint, const location&) const;

			uint return_address() const;

			void push(Symbol *s);

			Symbol* pop(const location&);

			void pop_frame();


		private:
			Interpreter *mp_interpreter;
			Context *mp_parent;
			std::vector<Symbol*> *mp_stack;
			uint m_return_address;
			uint m_frame_start;
			std::vector<Symbol*> m_temps;
			Symbol *mp_frame_reg;
			Symbol *mp_stack_reg;
			Symbol *mp_pc_reg;
			Symbol *mp_ra_reg;

			Context(const location&, Context*, uint, uint);

			Symbol* get_frame_reg();

			Symbol* get_stack_reg();

			Symbol* get_pc_reg();

			Symbol* get_ra_reg();
		};


		static const uint STACK_BASE;
		static const uint DYN_BASE;

		uint8_t m_options;
		Scanner *mp_scanner;
		SymbolTable *mp_table;
		MemoryManager *mp_memmngr;
		Parser *mp_parser;
		std::vector<Instruction> m_program;
		Context *mp_context;
		uint m_code_start;
		uint m_program_counter;


		bool compile(std::vector<Instruction*>*, std::list<Error>&);

		bool solve(Instruction*, std::list<Error>&);

		void execute();

		void warning(const location&, const std::string&);

		void general_logic_arithmetic(const Instruction&);

		void integer_logic_arithmetic(const Instruction&);

		void casting(const Instruction&);

		void move(const Instruction&);

		void branch_and_function(const Instruction&);

		Symbol* get_symbol(const Field&, const location&);

		Symbol* get_symbol(uint, const location&);

		uint get_addr(const Field&, const location&);

		Type::Kind get_type(const Field&, const location&);

		Field::Value get_val(const Field&, const location&);

		char get_cval(const Field&, const location&);

		int get_ival(const Field&, const location&);

		float get_fval(const Field&, const location&);

		void set_cval(const Field&, const location&, char v);

		void set_ival(const Field&, const location&, int v);

		void set_fval(const Field&, const location&, float v);

		Symbol* new_sym(const Field&, const location&);

		static Field::Value sym_to_field_val(Symbol*);

		static Symbol::Value field_to_sym_val(Type::Kind, Field::Value);
	};
}

#endif /* INTERPRETER_HPP_ */
