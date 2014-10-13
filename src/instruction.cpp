/**
 * @file instruction.cpp
 *
 * @brief
 *
 * @date 12/10/2014
 * @author Luciano Santos
 */

#include "instruction.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

namespace tac {
	Field::Field(const Symbol* s)
		: kind(s ? s->kind : Symbol::VAR),
		  type((s && s->type) ? s->type->kind : Type::CHAR),
		  solved(false) {

		value.referee = s;
	}

	Field::Field(const Field &other)
		: kind(other.kind),
		  type(other.type),
		  value(other.value),
		  solved(other.solved) {

		if ((!other.solved) && other.value.referee && (!other.value.referee->registered))
			value.referee = new Symbol(*other.value.referee);
	}

	Field& Field::operator=(const Field& other) {
		kind = other.kind;
		type = other.type;
		value = other.value;
		solved = other.solved;

		if ((!other.solved) && other.value.referee && (!other.value.referee->registered))
			value.referee = new Symbol(*other.value.referee);

		return *this;
	}

	Field::~Field() {
		if ((!solved) && value.referee && (!value.referee->registered))
			delete value.referee;
	}

	bool Field::solve(const SymbolTable &table) {
		if (solved)
			return true;
		if (!value.referee)
			return false;

		const Symbol *s;

		switch (value.referee->kind) {
		case Symbol::VAR:
		case Symbol::LABEL:
			s = table.get(*value.referee->id);
			if (!s) return false;
			if (!value.referee->registered)
				delete value.referee;

			kind = s->kind;
			if (kind == Symbol::LABEL)
				value.addrval = s->value.addrval;
			else {
				value.referee = s;
				if (s->type)
					type = s->type->kind;
			}
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

			switch (type) {
			case Type::CHAR:
				value.cval = s->value.cval;
				break;

			case Type::INT:
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

	std::string Field::to_str() const {
		std::ostringstream sstream;

		if (solved) {
			switch (kind) {
			case Symbol::VAR:
				sstream << "var " << *value.referee->id;
				break;

			case Symbol::LABEL:
				sstream << "addr " << value.addrval;
				break;

			case Symbol::PARAM:
				sstream << "param " << value.addrval;
				break;

			case Symbol::TEMP:
				sstream << "temp " << value.addrval;
				break;

			case Symbol::CONST:
				switch (type) {
				case Type::CHAR:
					sstream << "'";
					if (value.cval < 32)
						sstream << "\\" << (int) value.cval;
					else
						sstream << value.cval;
					sstream << "'";
					break;

				case Type::INT:
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
			 sstream << "refs " << value.referee->to_str();

		return sstream.str();
	}



	const uint8_t Instruction::ADD		= 0x01;
	const uint8_t Instruction::SUB		= 0x02;
	const uint8_t Instruction::MUL		= 0x03;
	const uint8_t Instruction::DIV		= 0x04;
	const uint8_t Instruction::AND		= 0x05;
	const uint8_t Instruction::OR		= 0x06;
	const uint8_t Instruction::BAND		= 0x07;
	const uint8_t Instruction::BOR		= 0x08;
	const uint8_t Instruction::BXOR		= 0x09;
	const uint8_t Instruction::SHL		= 0x0A;
	const uint8_t Instruction::SHR		= 0x0B;

	const uint8_t Instruction::SEQ		= 0x20;
	const uint8_t Instruction::SLT		= 0x21;
	const uint8_t Instruction::SLEQ		= 0x22;

	const uint8_t Instruction::MINUS	= 0x30;
	const uint8_t Instruction::NOT		= 0x31;
	const uint8_t Instruction::BNOT		= 0x32;

	const uint8_t Instruction::CHTOINT	= 0x41;
	const uint8_t Instruction::CHTOFL	= 0x42;
	const uint8_t Instruction::INTTOFL	= 0x43;
	const uint8_t Instruction::INTTOCH	= 0x44;
	const uint8_t Instruction::FLTOINT	= 0x45;
	const uint8_t Instruction::FLTOCH	= 0x46;

	const uint8_t Instruction::MOVVV	= 0x50;
	const uint8_t Instruction::MOVVD	= 0x51;
	const uint8_t Instruction::MOVVA	= 0x52;
	const uint8_t Instruction::MOVVI	= 0x53;
	const uint8_t Instruction::MOVDV	= 0x54;
	const uint8_t Instruction::MOVDD	= 0x55;
	const uint8_t Instruction::MOVDA	= 0x56;
	const uint8_t Instruction::MOVDI	= 0x57;
	const uint8_t Instruction::MOVIV	= 0x58;
	const uint8_t Instruction::MOVID	= 0x59;
	const uint8_t Instruction::MOVIA	= 0x5A;

	const uint8_t Instruction::BRZ		= 0x60;
	const uint8_t Instruction::BRNZ		= 0x61;
	const uint8_t Instruction::JUMP		= 0x62;
	const uint8_t Instruction::PARAM	= 0x63;
	const uint8_t Instruction::PRINT	= 0x64;
	const uint8_t Instruction::CALL		= 0x65;
	const uint8_t Instruction::RETURN	= 0x66;
	const uint8_t Instruction::PUSH		= 0x67;
	const uint8_t Instruction::POP		= 0x68;

	Instruction::Instruction(uint8_t opcode, Symbol *p0, Symbol *p1, Symbol *p2)
			: opcode(opcode) {
		operands[0] = Field(p0);
		operands[1] = Field(p1);
		operands[2] = Field(p2);
	}

	std::string Instruction::to_str() const {
		std::ostringstream sstream;

		sstream << std::hex << (int) opcode;

		if (operands[0].solved)
			sstream << ", " << operands[0].to_str();
		if (operands[1].solved)
			sstream << ", " << operands[1].to_str();
		if (operands[2].solved)
			sstream << ", " << operands[2].to_str();

		return sstream.str();
	}
}
