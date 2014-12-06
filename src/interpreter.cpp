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
 * @file interpreter.cpp
 *
 * @brief
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */

#include <fstream>
#include <iomanip>
#include <cstdlib>


#include "interpreter.hpp"
#define ON_ERROR(msg, p) errors.push_back(Error(ERROR, msg, *p.filename, p.line, p.column))
#define BOOL_TO_INT(a) ((a) ? 1 : 0)
#define BOOL_TO_INT_N(a) ((a) ? 0 : 1)

namespace tac
{

	TACExecutionException::TACExecutionException(const position &pos, const std::string &msg) throw()
		: TACException(pos), m_msg(msg) { }

	TACExecutionException::~TACExecutionException() throw() { }

	const std::string TACExecutionException::msg() const
	{
		return m_msg;
	}



	Interpreter::Context::Context(Interpreter *interpreter)
				: mp_interpreter(interpreter),
				  mp_parent(0),
				  mp_stack(new std::vector<Symbol*>()),
				  m_return_address(0),
				  m_frame_start(0),
				  mp_frame_reg(0),
				  mp_stack_reg(
						  new Symbol(new std::string("$s"), location(), Symbol::TEMP, new Type(Type::ADDR))),
				  mp_pc_reg(
						  new Symbol(new std::string("$pc"), location(), Symbol::TEMP, new Type(Type::ADDR))),
				  mp_ra_reg(0)
	{
		mp_frame_reg = new Symbol(new std::string("$f"), location(), Symbol::TEMP, new Type(Type::ADDR));
		mp_ra_reg = new Symbol(new std::string("$ra"), location(), Symbol::TEMP, new Type(Type::ADDR));
		mp_ra_reg->value.addrval = mp_frame_reg->value.addrval = mp_stack_reg->value.addrval = 0;
		mp_pc_reg->value.addrval = mp_interpreter->m_program_counter;
	}

	Interpreter::Context::Context(const location &loc, Context *parent, uint return_address, uint param_count)
			: mp_interpreter(parent->mp_interpreter),
			  mp_parent(parent),
			  mp_stack(parent->mp_stack),
			  m_return_address(return_address),
			  m_frame_start(parent->mp_stack->size() - param_count),
			  mp_frame_reg(0),
			  mp_stack_reg(parent->mp_stack_reg),
			  mp_pc_reg(parent->mp_pc_reg),
			  mp_ra_reg(0)
	{
		if (param_count > mp_stack->size())
			throw TACExecutionException(loc.begin, "number of parameters incompatible with stack size");

		mp_frame_reg = new Symbol(new std::string("$f"), location(), Symbol::TEMP, new Type(Type::ADDR));
		mp_frame_reg->value.addrval = m_frame_start;

		mp_ra_reg = new Symbol(new std::string("$ra"), location(), Symbol::TEMP, new Type(Type::ADDR));
		mp_ra_reg->value.addrval = m_return_address;
	}

	Interpreter::Context::~Context()
	{
		std::vector<Symbol*>::iterator i;
		if (!mp_parent)
		{
			for (i = mp_stack->begin(); i != mp_stack->end(); ++i)
				delete (*i);
			delete mp_stack;
			delete mp_stack_reg;
		}
		for (i = m_temps.begin(); i != m_temps.end(); ++i)
			delete (*i);
		delete mp_frame_reg;
	}

	Symbol* Interpreter::Context::get_frame_reg()
	{
		mp_frame_reg->type->kind = Type::ADDR;
		mp_frame_reg->type->array_size = 0;
		mp_frame_reg->value.addrval = m_frame_start;

		return mp_frame_reg;
	}

	Symbol* Interpreter::Context::get_stack_reg()
	{
		mp_stack_reg->type->kind = Type::ADDR;
		mp_stack_reg->type->array_size = 0;
		mp_stack_reg->value.addrval = mp_stack->empty() ? 0 : mp_stack->size() - 1;

		return mp_stack_reg;
	}

	Symbol* Interpreter::Context::get_pc_reg()
	{
		mp_pc_reg->type->kind = Type::ADDR;
		mp_pc_reg->type->array_size = 0;
		mp_pc_reg->value.addrval = mp_interpreter->m_program_counter;

		return mp_pc_reg;
	}

	Symbol* Interpreter::Context::get_ra_reg()
	{
		mp_ra_reg->type->kind = Type::ADDR;
		mp_ra_reg->type->array_size = 0;
		mp_ra_reg->value.addrval = m_return_address;

		return mp_ra_reg;
	}

	Interpreter::Context* Interpreter::Context::new_child(
			const location &loc,
			uint return_address,
			uint param_count)
	{
		return new Context(loc, this, return_address, param_count);
	}

	Interpreter::Context* Interpreter::Context::parent() const
	{
		return mp_parent;
	}

	Symbol* Interpreter::Context::get_param(uint id, const location &loc) const
	{
		return mp_stack->at(get_param_addr(id, loc));
	}

	Symbol* Interpreter::Context::get_temp(uint id, const location &loc)
	{
		if (id > FRAME_REG_CODE)
			throw TACExecutionException(loc.begin, "temporary's index is to large");

		if (id == FRAME_REG_CODE)
			return get_frame_reg();

		if (id == STACK_REG_CODE)
			return get_stack_reg();

		if (id == PC_REG_CODE)
			return get_pc_reg();

		if (id == RA_REG_CODE)
			return get_ra_reg();

		while (id >= m_temps.size())
			m_temps.push_back(new Symbol(0, loc, Symbol::TEMP, new Type(Type::INT)));
		return m_temps.at(id);
	}

	Symbol* Interpreter::Context::get(uint addr, const location &loc) const
	{
		if (addr < mp_stack->size())
			return mp_stack->at(addr);
		throw TACExecutionException(loc.begin, "invalid address access");
	}

	uint Interpreter::Context::get_param_addr(uint id, const location &loc) const
	{
		uint pos = m_frame_start + id;
		if (pos < mp_stack->size())
			return pos;
		throw TACExecutionException(loc.begin, "parameter out of stack bounds");
	}

	uint Interpreter::Context::return_address() const
	{
		return m_return_address;
	}

	void Interpreter::Context::push(Symbol *s)
	{
		mp_stack->push_back(s);
	}

	Symbol* Interpreter::Context::pop(const location &loc)
	{
		if (mp_stack->empty())
			throw TACExecutionException(loc.begin, "trying to pop empty stack");

		Symbol *s = mp_stack->back();
		mp_stack->pop_back();
		return s;
	}

	void Interpreter::Context::pop_frame()
	{
		while (m_frame_start < mp_stack->size())
		{
			delete mp_stack->back();
			mp_stack->pop_back();
		}
	}




	const uint Interpreter::STACK_REG_CODE = 0x400;
	const uint Interpreter::FRAME_REG_CODE = 0x401;
	const uint Interpreter::PC_REG_CODE = 0x402;
	const uint Interpreter::RA_REG_CODE = 0x403;
	const uint Interpreter::STACK_BASE = 0x55555555;
	const uint Interpreter::DYN_BASE = 0xAAAAAAAA;

	Interpreter::Interpreter(uint8_t opts)
		: m_options(opts),
		  mp_scanner(0),
		  mp_table(0),
		  mp_memmngr(0),
		  mp_parser(0),
		  mp_context(0),
		  m_code_start(0),
		  m_program_counter(0) { }

	Interpreter::~Interpreter()
	{
		delete mp_scanner;
		delete mp_table;
		delete mp_memmngr;
		delete mp_parser;
		delete mp_context;
	}

	void Interpreter::run(const char *in, std::list<Error> &errors)
	{
		std::ifstream in_file(in);
		if (!in_file.good())
		{
			errors.push_back(Error(ERROR, "couldn't open input file", in));
			return;
		}

		/* Parsers the input and generates a list of unsolved instructions. */
		std::vector<Instruction*> *unsolved = 0;

		delete (mp_scanner);
		mp_scanner = new Scanner(&in_file);

		delete (mp_table);
		mp_table = new SymbolTable(0x55555555);

		delete (mp_memmngr);
		mp_memmngr = new MemoryManager(0x55555555);

		delete (mp_parser);
		mp_parser = new Parser(*mp_scanner, mp_table, unsolved, m_code_start, in, errors);

		if (m_options & VERBOSE)
			std::cout << "parsing..." << std::endl;

		int result;
		if ((result = mp_parser->parse()))
		{
			if (result == 2)
				errors.push_back(Error(ERROR, "out of memory"));
			return;
		}
		/* If successful, tries to compile and run. */
		else if (compile(unsolved, errors))
		{
			try
			{
				execute();
			}
			catch (const TACExecutionException& e)
			{
				((Error) e).print(std::cerr);
			}
		}
	}

	/* Compiles a list of unsolved instructions into a program (list of solved instructions). */
	bool Interpreter::compile(std::vector<Instruction*> *instructions, std::list<Error> &errors)
	{
		if (m_options & VERBOSE)
			std::cout << "compiling..." << std::endl;

		bool result = true;

		/* Tries to solve each instruction, storing an error flag. */
		m_program.clear();
		m_program.reserve(instructions->size());
		for (std::vector<Instruction*>::iterator i = instructions->begin(); i != instructions->end(); ++i)
		{
			if ((*i) && solve(*i, errors))
				m_program.push_back(**i);
			else
				result = false;

			delete (*i);
		}
		delete instructions;

		return result;
	}

	/* Given an instruction, tries to solve it. */
	bool Interpreter::solve(Instruction *instr, std::list<Error> &errors)
	{
		/* First, are there any pending symbols in the operands? */
		for (int i = 0; i < 3; ++i)
		{
			if (instr->operands[i].value.referee && (!instr->operands[i].solve(*mp_table)))
			{
				const Symbol *s = instr->operands[i].value.referee;
				ON_ERROR("unresolved symbol '" + *s->id + "'", s->loc.begin);
				return false;
			}
		}

		/* Is the target a label? */
		if ((instr->opcode < 0x50) && (instr->operands[0].solved) && (instr->operands[0].kind == Symbol::LABEL))
		{
			ON_ERROR("invalid use of constant as operation target", instr->loc.begin);
			return false;
		}

		if (((instr->opcode == Instruction::MOVDA) ||
			(instr->opcode == Instruction::MOVIA) ||
			(instr->opcode == Instruction::MOVVA)) &&
				(instr->operands[1].kind == Symbol::LABEL))
		{
			ON_ERROR("invalid use of constant as target of address operator", instr->loc.begin);
			return false;
		}

		return true;
	}

	void Interpreter::execute()
	{
		if (m_options & DEBUG)
		{
			std::cout << "------------- Symbols -------------" << std::endl;
			mp_table->show();
			std::cout << "-------------- Code ---------------" << std::endl;
			uint k = m_code_start;
			for (std::vector<Instruction>::iterator i = m_program.begin(); i != m_program.end(); ++i, ++k)
			{
				std::cout << std::noshowbase << std::hex << std::setw(6) << std::setfill('0') << (int) k;
				std::cout << ": " << i->to_str() << std::endl;
			}
			std::cout << "-----------------------------------" << std::endl << std::endl;
		}

		if (m_options & VERBOSE)
			std::cout << "running..." << std::endl;

		srand(time(0));

		/* Creates root context. */
		mp_context = new Context(this);

		/* Initializes PC. */
		m_program_counter = m_code_start;
		const Symbol *s = mp_table->get("main");
		if (s && (s->kind == Symbol::LABEL))
			m_program_counter = s->value.addrval;

		uint limit = m_code_start + m_program.size();
		while (m_program_counter < limit)
		{
			const Instruction &i = m_program.at(m_program_counter - m_code_start);

			if (m_options & STEP)
			{
				std::cout
					<< std::noshowbase << std::hex << std::setw(6) << std::setfill('0')
					<< (int) m_program_counter;
				std::cout << ": " << i.to_str() << std::endl;
			}

			switch (i.opcode & 0xF0)
			{
			case 0x00: general_logic_arithmetic(i); break;
			case 0x10: integer_logic_arithmetic(i); break;
			case 0x20: casting(i); break;
			case 0x30: move(i); break;
			default: branch_and_function(i); break;
			}
		}
	}

	void Interpreter::warning(const location &loc, const std::string &msg)
	{
		Error e(WARNING, msg, *loc.begin.filename, loc.begin.line, loc.begin.column);
		e.print(std::cout);
		std::cout << std::endl;
	}

	void Interpreter::general_logic_arithmetic(const Instruction &i)
	{
		const Field &target = i.operands[0];

		Type::Kind type = get_type(i.operands[1], i.loc);
		if (target.kind != Symbol::TEMP)
			type = get_type(target, i.loc);

		for (int j = 1; (j < 3) && i.operands[j].solved; ++j)
			if (get_type(i.operands[j], i.loc) != type)
				warning(i.loc, "different types for target and operands");

		const Field &op1 = i.operands[1];
		const Field &op2 = i.operands[2];
		switch (i.opcode)
		{
		case Instruction::ADD:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, get_cval(op1, i.loc) + get_cval(op2, i.loc));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, get_ival(op1, i.loc) + get_ival(op2, i.loc));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, get_fval(op1, i.loc) + get_fval(op2, i.loc));
				break;
			}
			break;

		case Instruction::SUB:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, get_cval(op1, i.loc) - get_cval(op2, i.loc));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, get_ival(op1, i.loc) - get_ival(op2, i.loc));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, get_fval(op1, i.loc) - get_fval(op2, i.loc));
				break;
			}
			break;

		case Instruction::MUL:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, get_cval(op1, i.loc) * get_cval(op2, i.loc));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, get_ival(op1, i.loc) * get_ival(op2, i.loc));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, get_fval(op1, i.loc) * get_fval(op2, i.loc));
				break;
			}
			break;

		case Instruction::DIV:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, get_cval(op1, i.loc) / get_cval(op2, i.loc));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, get_ival(op1, i.loc) / get_ival(op2, i.loc));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, get_fval(op1, i.loc) / get_fval(op2, i.loc));
				break;
			}
			break;

		case Instruction::AND:
			switch (type)
			{
			case Type::CHAR:
				set_cval(
					target,
					i.loc,
					BOOL_TO_INT(
						BOOL_TO_INT(get_cval(op1, i.loc)) &&
						BOOL_TO_INT(get_cval(op2, i.loc))));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(
					target,
					i.loc,
					BOOL_TO_INT(
						BOOL_TO_INT(get_cval(op1, i.loc)) &&
						BOOL_TO_INT(get_cval(op2, i.loc))));
				break;
			case Type::FLOAT:
				set_fval(
					target,
					i.loc,
					BOOL_TO_INT(
						BOOL_TO_INT(get_cval(op1, i.loc)) &&
						BOOL_TO_INT(get_cval(op2, i.loc))));
				break;
			}
			break;

		case Instruction::OR:
			switch (type)
			{
			case Type::CHAR:
				set_cval(
					target,
					i.loc,
					BOOL_TO_INT(
						BOOL_TO_INT(get_cval(op1, i.loc)) ||
						BOOL_TO_INT(get_cval(op2, i.loc))));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(
					target,
					i.loc,
					BOOL_TO_INT(
						BOOL_TO_INT(get_cval(op1, i.loc)) ||
						BOOL_TO_INT(get_cval(op2, i.loc))));
				break;
			case Type::FLOAT:
				set_fval(
					target,
					i.loc,
					BOOL_TO_INT(
						BOOL_TO_INT(get_cval(op1, i.loc)) ||
						BOOL_TO_INT(get_cval(op2, i.loc))));
				break;
			}
			break;

		case Instruction::MINUS:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, -get_cval(op1, i.loc));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, -get_ival(op1, i.loc));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, -get_fval(op1, i.loc));
				break;
			}
			break;

		case Instruction::NOT:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, BOOL_TO_INT_N(get_cval(op1, i.loc)));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, BOOL_TO_INT_N(get_ival(op1, i.loc)));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, BOOL_TO_INT_N(get_fval(op1, i.loc)));
				break;
			}
			break;

		case Instruction::SEQ:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, BOOL_TO_INT(get_cval(op1, i.loc) == get_cval(op2, i.loc)));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, BOOL_TO_INT(get_ival(op1, i.loc) == get_ival(op2, i.loc)));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, BOOL_TO_INT(get_fval(op1, i.loc) == get_fval(op2, i.loc)));
				break;
			}
			break;

		case Instruction::SLT:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, BOOL_TO_INT(get_cval(op1, i.loc) < get_cval(op2, i.loc)));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, BOOL_TO_INT(get_ival(op1, i.loc) < get_ival(op2, i.loc)));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, BOOL_TO_INT(get_fval(op1, i.loc) < get_fval(op2, i.loc)));
				break;
			}
			break;

		case Instruction::SLEQ:
			switch (type)
			{
			case Type::CHAR:
				set_cval(target, i.loc, BOOL_TO_INT(get_cval(op1, i.loc) <= get_cval(op2, i.loc)));
				break;
			case Type::INT:
			case Type::ADDR:
				set_ival(target, i.loc, BOOL_TO_INT(get_ival(op1, i.loc) <= get_ival(op2, i.loc)));
				break;
			case Type::FLOAT:
				set_fval(target, i.loc, BOOL_TO_INT(get_fval(op1, i.loc) <= get_fval(op2, i.loc)));
				break;
			}
			break;
		}

		if (target.kind == Symbol::TEMP)
			get_symbol(target, i.loc)->type->kind = type;

		++m_program_counter;
	}

	void Interpreter::integer_logic_arithmetic(const Instruction &i)
	{
		const Field &target = i.operands[0];
		Type::Kind type = get_type(target, i.loc);

		if (type != Type::INT)
		{
			if (target.kind == Symbol::TEMP)
				type = Type::INT;
			else
				warning(i.loc, "target of integer operation is not an integer");
		}
		for (int j = 1; (j < 3) && i.operands[j].solved; ++j)
			if (get_type(i.operands[j], i.loc) != type)
				warning(i.loc, "different types for target and operands");

		const Field &op1 = i.operands[1];
		const Field &op2 = i.operands[2];
		switch (i.opcode)
		{
		case Instruction::BAND:
			set_ival(target, i.loc, get_ival(op1, i.loc) & get_ival(op2, i.loc));
			break;

		case Instruction::BOR:
			set_ival(target, i.loc, get_ival(op1, i.loc) | get_ival(op2, i.loc));
			break;

		case Instruction::BXOR:
			set_ival(target, i.loc, get_ival(op1, i.loc) ^ get_ival(op2, i.loc));
			break;

		case Instruction::BNOT:
			set_ival(target, i.loc, ~get_ival(op1, i.loc));
			break;

		case Instruction::SHL:
			set_ival(target, i.loc, get_ival(op1, i.loc) << get_ival(op2, i.loc));
			break;

		case Instruction::SHR:
			set_ival(target, i.loc, get_ival(op1, i.loc) >> get_ival(op2, i.loc));
			break;

		case Instruction::MOD:
			set_ival(target, i.loc, get_ival(op1, i.loc) % get_ival(op2, i.loc));
			break;
		}

		if (target.kind == Symbol::TEMP)
			type = get_symbol(target, i.loc)->type->kind = Type::INT;

		++m_program_counter;
	}

	void Interpreter::casting(const Instruction &i)
	{
		Type::Kind target_t = (Type::Kind) (i.opcode & 0x03);
		Type::Kind source_t = (Type::Kind) ((i.opcode & 0x0C) >> 2);
		const Field &target = i.operands[0];
		const Field &source = i.operands[1];

		Type::Kind type = target_t;
		if (target.kind != Symbol::TEMP)
			type = get_type(target, i.loc);

		if (type != target_t)
			warning(i.loc, "divergent type for target of cast");
		if (get_type(source, i.loc) != source_t)
			warning(i.loc, "divergent type for source of cast");

		switch (target_t)
		{
		case Type::CHAR:
			switch (source_t)
			{
			case Type::INT:
				set_cval(target, i.loc, (char) get_ival(source, i.loc));
				break;
			case Type::FLOAT:
				set_cval(target, i.loc, (char) get_fval(source, i.loc));
				break;
			default:
				break;
			}
			break;

		case Type::INT:
			switch (source_t)
			{
			case Type::CHAR:
				set_ival(target, i.loc, (int) get_cval(source, i.loc));
				break;
			case Type::FLOAT:
				set_ival(target, i.loc, (int) get_fval(source, i.loc));
				break;
			default:
				break;
			}
			break;

		case Type::FLOAT:
			switch (source_t)
			{
			case Type::CHAR:
				set_fval(target, i.loc, (float) get_cval(source, i.loc));
				break;
			case Type::INT:
				set_fval(target, i.loc, (float) get_ival(source, i.loc));
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}

		if (target.kind == Symbol::TEMP)
			get_symbol(target, i.loc)->type->kind = target_t;

		++m_program_counter;
	}

	void Interpreter::move(const Instruction &i)
	{
		uint8_t src_mode = i.opcode & 0x03;
		uint8_t tgt_mode = (i.opcode & 0x0C) >> 2;

		const Field &tgt_op = i.operands[0];
		Symbol* tgt_sym = get_symbol(i.operands[0], i.loc);

		const Field &src_op = i.operands[1];
		Type::Kind src_type;
		Field::Value src_val;
		memset(&src_val, 0, sizeof(src_val));

		// Read value from the source...
		switch (src_mode)
		{
		case 0: //value
			src_type = get_type(src_op, i.loc);
			src_val = get_val(src_op, i.loc);
			break;

		case 1: //deref
		case 3: //index
			if (get_type(src_op, i.loc) != Type::ADDR)
				warning(i.loc, "dereferencing a non-pointer value");
			{
				uint addr = get_val(src_op, i.loc).addrval;
				if (src_mode == 3)
				{
					const Field &offset = i.operands[2];
					if (get_type(offset, i.loc) != Type::INT)
						warning(i.loc, "non-integer array index");
					addr += get_ival(offset, i.loc);
				}

				Symbol *s = get_symbol(addr, i.loc);
				src_type = s->type->kind;
				src_val = sym_to_field_val(s);
			}
			break;

		case 2: //address
			src_type = Type::ADDR;
			src_val.addrval = get_addr(src_op, i.loc);
			break;
		}


		// Writes value at target.

		if (tgt_mode != 0) // 1-deref or 3-index
		{
			if (get_type(tgt_op, i.loc) != Type::ADDR)
				warning(i.loc, "dereferencing a non-pointer value");
			uint addr = get_val(tgt_op, i.loc).addrval;
			if (tgt_mode == 3)
			{
				const Field &offset = i.operands[2];
				if (get_type(offset, i.loc) != Type::INT)
					warning(i.loc, "non-integer array index");
				addr += get_ival(offset, i.loc);
			}
			tgt_sym = get_symbol(addr, i.loc);
		}

		if (tgt_sym->kind == Symbol::TEMP)
			tgt_sym->type->kind = src_type;
		else if (tgt_sym->type->kind != src_type)
			warning(i.loc, "divergent type for target of move");
		tgt_sym->value = field_to_sym_val(src_type, src_val);

		++m_program_counter;
	}

	void Interpreter::branch_and_function(const Instruction &i)
	{
		const Field &target = i.operands[0];
		const Field &op = i.operands[1];

		// moves next by default...
		++m_program_counter;

		switch (i.opcode) {
		case Instruction::BRZ:
		case Instruction::BRNZ:
		case Instruction::JUMP:
		case Instruction::CALL:
			{
				if (get_type(target, i.loc) != Type::ADDR)
					warning(i.loc, "non address value used as address");

				bool jump = true;
				if ((i.opcode == Instruction::BRZ) || (i.opcode == Instruction::BRNZ))
				{
					switch (get_type(op, i.loc))
					{
					case Type::CHAR:
						jump = get_cval(op, i.loc) == 0;
						break;

					case Type::ADDR:
					case Type::INT:
						jump = get_ival(op, i.loc) == 0;
						break;

					case Type::FLOAT:
						jump = get_fval(op, i.loc) == 0;
						break;
					}

					if (i.opcode == Instruction::BRNZ)
						jump = !jump;
				}

				if (jump)
				{
					uint addr = target.value.addrval;
					uint limit = m_code_start + m_program.size();
					if ((addr < m_code_start) || (addr >= limit))
						throw TACExecutionException(i.loc.begin, "jump to an invalid code address");

					if (i.opcode == Instruction::CALL)
						mp_context = mp_context->new_child(i.loc, m_program_counter, (uint) op.value.ival);
					m_program_counter = addr;
				}
			}
			break;

		case Instruction::RETURN:
			{
				Context *parent = mp_context->parent();
				if (!parent)
					throw TACExecutionException(i.loc.begin, "returning to nowhere");

				uint ra = mp_context->return_address();
				Symbol *s = 0;
				if (target.solved)
					s = new_sym(target, i.loc);

				mp_context->pop_frame();
				delete mp_context;
				mp_context = parent;
				if (s) mp_context->push(s);
				m_program_counter = ra;
			}
			break;

		case Instruction::PARAM:
		case Instruction::PUSH:
			mp_context->push(new_sym(target, i.loc));
			break;

		case Instruction::POP:
			{
				Symbol *src = mp_context->pop(i.loc);
				Symbol *tgt_sym = get_symbol(target, i.loc);
				if (target.kind == Symbol::TEMP)
					tgt_sym->type->kind = src->type->kind;
				else if (tgt_sym->type->kind != src->type->kind)
					warning(i.loc, "divergent type for target symbol");
				tgt_sym->value = src->value;
			}
			break;

		case Instruction::PRINT:
		case Instruction::PRINTLN:
			if (target.solved)
			{
				Type::Kind type = get_type(target, i.loc);
				Field::Value v = get_val(target, i.loc);
				std::cout << std::dec;
				switch (type)
				{
				case Type::CHAR: std::cout << v.cval; break;
				case Type::INT: std::cout << v.ival; break;
				case Type::FLOAT: std::cout << v.fval; break;
				case Type::ADDR:
					std::cout
						<< "0x"
						<< std::noshowbase << std::hex << std::setw(6) << std::setfill('0')
						<< (int) v.addrval;
					break;
				}
			}
			if (i.opcode ==  Instruction::PRINTLN)
				std::cout << std::endl;
			break;

		case Instruction::SCANC:
			{
				char num;
				std::cin >> num;
				Symbol *tgt_sym = get_symbol(target, i.loc);
				if (target.kind == Symbol::TEMP)
					tgt_sym->type->kind = Type::CHAR;
				else if (tgt_sym->type->kind != Type::CHAR)
					warning(i.loc, "divergent type for target symbol");
				tgt_sym->value.cval = num;
			}
			break;

		case Instruction::SCANI:
			{
				int num;
				std::cin >> num;
				Symbol *tgt_sym = get_symbol(target, i.loc);
				if (target.kind == Symbol::TEMP)
					tgt_sym->type->kind = Type::INT;
				else if (tgt_sym->type->kind != Type::INT)
					warning(i.loc, "divergent type for target symbol");
				tgt_sym->value.ival = num;
			}
			break;

		case Instruction::SCANF:
			{
				float num;
				std::cin >> num;
				Symbol *tgt_sym = get_symbol(target, i.loc);
				if (target.kind == Symbol::TEMP)
					tgt_sym->type->kind = Type::FLOAT;
				else if (tgt_sym->type->kind != Type::FLOAT)
					warning(i.loc, "divergent type for target symbol");
				tgt_sym->value.fval = num;
			}
			break;

		case Instruction::MEMA:
			{
				Symbol *tgt_sym = get_symbol(target, i.loc);
				if (target.kind == Symbol::TEMP)
					tgt_sym->type->kind = Type::ADDR;
				else if (tgt_sym->type->kind != Type::ADDR)
					warning(i.loc, "divergent type for target symbol");

				if (get_type(op, i.loc) != Type::INT)
					warning(i.loc, "non-integer array index");
				uint size = (uint) get_ival(op, i.loc);
				uint addr;
				tgt_sym->value.addrval =  (mp_memmngr->alloc(size, addr)) ? (addr + DYN_BASE) : 0;
			}
			break;

		case Instruction::MEMF:
			{
				Type::Kind type = get_type(target, i.loc);
				Field::Value v = get_val(target, i.loc);
				if (type != Type::ADDR)
					warning(i.loc, "trying to free a non-address value");
				mp_memmngr->free(v.addrval - DYN_BASE);
			}
			break;

		case Instruction::RAND:
			{
				Symbol *tgt_sym = get_symbol(target, i.loc);
				if (target.kind == Symbol::TEMP)
					tgt_sym->type->kind = Type::INT;
				else if (tgt_sym->type->kind != Type::INT)
					warning(i.loc, "divergent type for target symbol");
				tgt_sym->value.ival = rand() % 2147483647;
			}
			break;
		}
	}

	Symbol* Interpreter::get_symbol(const Field &field, const location &loc)
	{
		if (field.kind == Symbol::PARAM)
			return mp_context->get_param(field.value.addrval, loc);
		else if (field.kind == Symbol::TEMP)
			return mp_context->get_temp(field.value.addrval, loc);
		else
			return const_cast<Symbol*>(field.value.referee);
	}

	Symbol* Interpreter::get_symbol(uint addr, const location &loc)
	{
		if (addr < STACK_BASE)
		{
			Symbol *s = const_cast<Symbol*>(mp_table->get(addr));
			if (!s)
				throw TACExecutionException(loc.begin, "invalid address access");
			return s;
		}
		else if (addr < DYN_BASE)
			return mp_context->get(addr - STACK_BASE, loc);
		else
		{
			const Symbol *s = mp_memmngr->get(addr - DYN_BASE);
			if (!s)
				throw TACExecutionException(loc.begin, "invalid address access");
			return const_cast<Symbol*>(s);
		}
	}

	uint Interpreter::get_addr(const Field &field, const location &loc)
	{
		return (field.kind == Symbol::VAR) ?
				mp_table->get_addr(*field.value.referee->id) :
				mp_context->get_param_addr(field.value.addrval, loc) + STACK_BASE;
	}

	Type::Kind Interpreter::get_type(const Field &field, const location &loc)
	{
		switch (field.kind)
		{
		case Symbol::CONST:
		case Symbol::LABEL:
			return field.type;

		default:
			return get_symbol(field, loc)->type->kind;
		}
	}

	Field::Value Interpreter::get_val(const Field &field, const location &loc)
	{
		switch (field.kind)
		{
		case Symbol::CONST:
		case Symbol::LABEL:
			return field.value;

		default:
			Symbol *s = get_symbol(field, loc);
			if (s->type->array_size)
				s = s->value.arrval->front();
			return sym_to_field_val(s);
		}
	}

	char Interpreter::get_cval(const Field &field, const location &loc)
	{
		Field::Value v = get_val(field, loc);
		Type::Kind t = get_type(field, loc);

		switch (t)
		{
		case Type::CHAR: return v.cval;
		case Type::INT: return (char) v.ival;
		case Type::FLOAT: return (char) v.fval;
		case Type::ADDR: return (char) v.addrval;
		}
		return 0;
	}

	int Interpreter::get_ival(const Field &field, const location &loc)
	{
		Field::Value v = get_val(field, loc);
		Type::Kind t = get_type(field, loc);

		switch (t)
		{
		case Type::CHAR: return (int) v.cval;
		case Type::INT: return v.ival;
		case Type::FLOAT: return (int) v.fval;
		case Type::ADDR: return (int) v.addrval;
		}
		return 0;
	}

	float Interpreter::get_fval(const Field &field, const location &loc)
	{
		Field::Value v = get_val(field, loc);
		Type::Kind t = get_type(field, loc);

		switch (t)
		{
		case Type::CHAR: return (float) v.cval;
		case Type::INT: return (float) v.ival;
		case Type::FLOAT: return v.fval;
		case Type::ADDR: return (float) v.addrval;
		}
		return 0;
	}

	void Interpreter::set_cval(const Field &field, const location &loc, char v)
	{
		Symbol *s = get_symbol(field, loc);
		if (s->type->array_size)
			s = s->value.arrval->front();
		s->value.cval = v;
	}

	void Interpreter::set_ival(const Field &field, const location &loc, int v)
	{
		Symbol *s = get_symbol(field, loc);
		if (s->type->array_size)
			s = s->value.arrval->front();
		s->value.ival = v;
	}

	void Interpreter::set_fval(const Field &field, const location &loc, float v)
	{
		Symbol *s = get_symbol(field, loc);
		if (s->type->array_size)
			s = s->value.arrval->front();
		s->value.fval = v;
	}

	Symbol* Interpreter::new_sym(const Field &field, const location &loc)
	{
		Symbol *s = new Symbol(0, loc, Symbol::CONST, new Type(field.type));
		Symbol *s2;

		switch (field.kind)
		{
		case Symbol::CONST:
		case Symbol::LABEL:
			s->value = field_to_sym_val(field.type, field.value);
			break;

		default:
			s2 = get_symbol(field, loc);
			s->value = s2->value;
			s->type->kind = s2->type->kind;
			break;
		}

		return s;
	}

	Field::Value Interpreter::sym_to_field_val(Symbol *s)
	{
		Field::Value v;
		memset(&v, 0, sizeof(v));

		switch (s->type->kind)
		{
		case Type::CHAR: v.cval = s->value.cval; break;
		case Type::INT: v.ival = s->value.ival; break;
		case Type::FLOAT: v.fval = s->value.fval; break;
		case Type::ADDR: v.addrval = s->value.addrval; break;
		}

		return v;
	}

	Symbol::Value Interpreter::field_to_sym_val(Type::Kind type, Field::Value val)
	{
		Symbol::Value v;
		memset(&v, 0, sizeof(v));

		switch (type)
		{
		case Type::CHAR: v.cval = val.cval; break;
		case Type::INT: v.ival = val.ival; break;
		case Type::FLOAT: v.fval = val.fval; break;
		case Type::ADDR: v.addrval = val.addrval; break;
		}

		return v;
	}
}
