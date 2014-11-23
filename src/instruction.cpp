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
 * @file instruction.cpp
 *
 * @brief
 *
 * @date 2014-10-12
 *
 * @author Luciano Santos
 */

#include "instruction.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

namespace tac
{
	Field::Field(const Symbol* s)
		: kind(s ? s->kind : Symbol::VAR),
		  type((s && s->type) ? s->type->kind : Type::CHAR),
		  solved(false)
	{
		value.referee = s;
	}

	Field::Field(const Field &other)
		: kind(other.kind),
		  type(other.type),
		  value(other.value),
		  solved(other.solved)
	{
		if ((!other.solved) && other.value.referee && (!other.value.referee->registered))
			value.referee = new Symbol(*other.value.referee);
	}

	Field& Field::operator=(const Field& other)
	{
		kind = other.kind;
		type = other.type;
		value = other.value;
		solved = other.solved;

		if ((!other.solved) && other.value.referee && (!other.value.referee->registered))
			value.referee = new Symbol(*other.value.referee);

		return *this;
	}

	Field::~Field()
	{
		if ((!solved) && value.referee && (!value.referee->registered))
			delete value.referee;
	}

	bool Field::solve(const SymbolTable &table)
	{
		if (solved)
			return true;
		if (!value.referee)
			return false;

		const Symbol *s;

		switch (value.referee->kind)
		{
		case Symbol::VAR:
		case Symbol::LABEL:
			s = table.get(*value.referee->id);
			if (!s) return false;
			if (!value.referee->registered)
				delete value.referee;

			kind = s->kind;
			if (s->type)
				type = s->type->kind;
			if (kind == Symbol::LABEL)
				value.addrval = s->value.addrval;
			else
				value.referee = s;
			break;

		case Symbol::PARAM:
		case Symbol::TEMP:
			s = value.referee;
			kind = s->kind;
			value.addrval = s->value.addrval;
			if (!s->registered)
				delete s;
			break;

		case Symbol::CONST:
			s = value.referee;
			kind = s->kind;
			type = s->type->kind;

			switch (type)
			{
			case Type::CHAR:
				value.cval = s->value.cval;
				break;

			case Type::INT:
			case Type::ADDR:
				value.ival = s->value.ival;
				break;

			case Type::FLOAT:
				value.fval = s->value.fval;
				break;
			}

			if (!s->registered)
				delete s;
			break;
		}

		solved = true;
		return true;
	}

	std::string Field::to_str() const
	{
		std::ostringstream sstream;

		if (solved)
		{
			switch (kind)
			{
			case Symbol::VAR:
				sstream << *value.referee->id;
				break;

			case Symbol::LABEL:
				sstream
					<< "0x"
					<< std::noshowbase << std::hex << std::setw(8) << std::setfill('0')
					<< (int) value.addrval;
				break;

			case Symbol::PARAM:
				sstream << "#" << value.addrval;
				break;

			case Symbol::TEMP:
				sstream << "$" << value.addrval;
				break;

			case Symbol::CONST:
				switch (type)
				{
				case Type::CHAR:
					sstream << "'";
					if (value.cval < 32)
						sstream << "\\" << (int) value.cval;
					else
						sstream << value.cval;
					sstream << "'";
					break;

				case Type::INT:
				case Type::ADDR:
					sstream << value.ival;
					break;

				case Type::FLOAT:
					sstream << value.fval;
					break;
				}
				break;
			}
		}
		else if (value.referee)
			 sstream << "ref " << value.referee->to_str();

		return sstream.str();
	}



	Instruction::Instruction(const location& loc, OpCode opcode, Symbol *p0, Symbol *p1, Symbol *p2)
			: loc(loc), opcode(opcode)
	{
		operands[0] = Field(p0);
		operands[1] = Field(p1);
		operands[2] = Field(p2);
	}

	std::string Instruction::to_str() const
	{
		std::ostringstream sstream;

		sstream << opname(opcode);

		if (operands[0].solved)
			sstream << " " << operands[0].to_str();
		if (operands[1].solved)
			sstream << ", " << operands[1].to_str();
		if (operands[2].solved)
			sstream << ", " << operands[2].to_str();

		return sstream.str();
	}

	std::string Instruction::opname(uint8_t op)
	{
		switch (op)
		{
		case ADD:	  return "add";
		case SUB:	  return "sub";
		case MUL:	  return "mul";
		case DIV:	  return "div";
		case AND:	  return "and";
		case OR:	  return "or";
		case MINUS:	  return "minus";
		case NOT:	  return "not";
		case SEQ:	  return "seq";
		case SLT:	  return "slt";
		case SLEQ:	  return "sleq";
		case BAND:	  return "band";
		case BOR:	  return "bor";
		case BXOR:	  return "bxor";
		case SHL:	  return "shl";
		case SHR:	  return "shr";
		case BNOT:	  return "bnot";
		case MOD:	  return "mod";
		case CHTOINT: return "chtoint";
		case CHTOFL:  return "chtofl";
		case INTTOCH: return "inttoch";
		case INTTOFL: return "inttofl";
		case FLTOCH:  return "fltoch";
		case FLTOINT: return "fltoint";
		case MOVVV:	  return "movvv";
		case MOVVD:	  return "movvd";
		case MOVVA:	  return "movva";
		case MOVVI:	  return "movvi";
		case MOVDV:	  return "movdv";
		case MOVDD:	  return "movdd";
		case MOVDA:	  return "movda";
		case MOVDI:	  return "movdi";
		case MOVIV:	  return "moviv";
		case MOVID:	  return "movid";
		case MOVIA:	  return "movia";
		case POP:	  return "pop";
		case BRZ:	  return "brz";
		case BRNZ:	  return "brnz";
		case JUMP:	  return "jump";
		case PARAM:	  return "param";
		case PRINT:	  return "print";
		case PRINTLN: return "println";
		case SCANC:   return "scanc";
		case SCANI:   return "scani";
		case SCANF:   return "scanf";
		case RAND:    return "rand";
		case MEMA:    return "mema";
		case MEMF:    return "memf";
		case CALL: 	  return "call";
		case RETURN:  return "return";
		case PUSH:	  return "push";
		default:
			std::ostringstream s;
			s << "0x"
				<< std::noshowbase << std::hex << std::setw(2) << std::setfill('0')
				<< (int) op;
			return s.str();
		}
	}
}
