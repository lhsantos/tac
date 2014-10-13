/**
 * @file instruction.hpp
 *
 * @brief
 *
 * @date 12/10/2014
 * @author Luciano Santos
 */

#ifndef INSTRUCTION_HPP_
#define INSTRUCTION_HPP_ 1

#include <list>
#include <string>

#include "table.hpp"


namespace tac {
	/**
	 * The field of an instruction, which may be any kind of symbol.
	 */
	struct Field {
	public:
		/**
		 * If the field refers to a variable or is not yet resolved, referee will hold its value, otherwise,
		 * labels, parameters and temporaries will use addrval and constants will use the remaining fields.
		 */
		union Value {
			const Symbol *referee;
			uint addrval;
			int ival;
			char cval;
			float fval;
		};

		Symbol::Kind kind;
		Type::Kind type;
		Value value;
		bool solved;

		Field(const Symbol* s = 0);

		Field(const Field&);

		Field& operator=(const Field&);

		~Field();

		bool solve(const SymbolTable&);

		std::string to_str() const;
	};


	/**
	 * A simple three address code instruction.
	 */
	struct Instruction {
	public:
		static const uint8_t ADD;
		static const uint8_t SUB;
		static const uint8_t MUL;
		static const uint8_t DIV;
		static const uint8_t AND;
		static const uint8_t OR;
		static const uint8_t BAND;
		static const uint8_t BOR;
		static const uint8_t BXOR;
		static const uint8_t SHL;
		static const uint8_t SHR;
		static const uint8_t SEQ;
		static const uint8_t SLT;
		static const uint8_t SLEQ;
		static const uint8_t MINUS;
		static const uint8_t NOT;
		static const uint8_t BNOT;
		static const uint8_t CHTOINT;
		static const uint8_t CHTOFL;
		static const uint8_t INTTOFL;
		static const uint8_t INTTOCH;
		static const uint8_t FLTOINT;
		static const uint8_t FLTOCH;
		static const uint8_t BRZ;
		static const uint8_t BRNZ;
		static const uint8_t MOVVV;
		static const uint8_t MOVVD;
		static const uint8_t MOVVA;
		static const uint8_t MOVVI;
		static const uint8_t MOVDV;
		static const uint8_t MOVDD;
		static const uint8_t MOVDA;
		static const uint8_t MOVDI;
		static const uint8_t MOVIV;
		static const uint8_t MOVID;
		static const uint8_t MOVIA;
		static const uint8_t JUMP;
		static const uint8_t PARAM;
		static const uint8_t PRINT;
		static const uint8_t CALL;
		static const uint8_t RETURN;
		static const uint8_t PUSH;
		static const uint8_t POP;

		uint8_t opcode;
		Field operands[3];

		Instruction(uint8_t opcode, Symbol* p0 = 0, Symbol* p1 = 0, Symbol* p2 = 0);

		std::string to_str() const;
	};
}

#endif /* INSTRUCTION_HPP_ */
