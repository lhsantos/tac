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


namespace tac
{
	/**
	 * The field of an instruction, which may be any kind of symbol.
	 */
	struct Field
	{
	public:
		/**
		 * If the field refers to a variable or is not yet resolved, referee will hold its value, otherwise,
		 * labels, parameters and temporaries will use addrval and constants will use the remaining fields.
		 */
		union Value
		{
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
	struct Instruction
	{
	public:
		enum OpCode
		{
			ADD		= 0x00,
			SUB		= 0x01,
			MUL		= 0x02,
			DIV		= 0x03,
			AND		= 0x04,
			OR		= 0x05,
			MINUS	= 0x06,
			NOT		= 0x07,
			SEQ		= 0x08,
			SLT		= 0x09,
			SLEQ	= 0x0A,

			BAND	= 0x10,
			BOR		= 0x11,
			BXOR	= 0x12,
			SHL		= 0x13,
			SHR		= 0x14,
			BNOT	= 0x15,

			CHTOINT	= 0x20, // 0000
			CHTOFL	= 0x22, // 0010
			INTTOCH	= 0x24, // 0100
			INTTOFL	= 0x26, // 0110
			FLTOCH	= 0x28, // 1000
			FLTOINT	= 0x29, // 1001

			MOVVV	= 0x30, // 0000
			MOVVD	= 0x31, // 0001
			MOVVA	= 0x32, // 0010
			MOVVI	= 0x33, // 0011
			MOVDV	= 0x34, // 0100
			MOVDD	= 0x35, // 0101
			MOVDA	= 0x36, // 0110
			MOVDI	= 0x37, // 0111
			MOVIV	= 0x3C, // 1100
			MOVID	= 0x3D, // 1101
			MOVIA	= 0x3E, // 1110

			POP		= 0x40,

			BRZ		= 0x50,
			BRNZ	= 0x51,
			JUMP	= 0x52,
			PARAM	= 0x53,
			PRINT	= 0x54,
			PRINTLN	= 0x55,
			CALL	= 0x56,
			RETURN	= 0x57,
			PUSH	= 0x58
		};

		location loc;
		uint8_t opcode;
		Field operands[3];

		Instruction(const location& loc, OpCode opcode, Symbol* p0 = 0, Symbol* p1 = 0, Symbol* p2 = 0);

		std::string to_str() const;

	private:
		static std::string opname(uint8_t);
	};
}

#endif /* INSTRUCTION_HPP_ */
