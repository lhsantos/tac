/**
 * @file table.hpp
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */

#ifndef TABLE_HPP_
#define TABLE_HPP_ 1

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
#endif

#include <map>
#include <list>
#include <string>
#include <vector>

#include "location.hh"

namespace tac
{
	struct Symbol;
	class SymbolTable;

	struct Type
	{
	public:
		enum Kind
		{
			CHAR  = 0,
			INT   = 1,
			FLOAT = 2,
			ADDR  = 3
		};

		Kind kind; // the language type
		size_t array_size;

		/**
		 * @brief Creates a copy of a type.
		 *
		 * The pointee will be duplicated, i.e., a newly created object will be associated to this type,
		 * so both objects can be destroyed independently.
		 *
		 * @param other
		 */
		Type(const Type& other);

		/**
		 * @brief Creates a new type descriptor, with given values.
		 *
		 * The name string will not be destroyed, the pointee will, even if it was allocated outside this class.
		 *
		 * @param kind
		 * @param constant
		 * @param pointee
		 * @param array_size
		 */
		Type(Kind kind, size_t array_size = 0);

		/**
		 * @brief Destroys this type, and recursively destroys its pointee.
		 */
		~Type();

		std::string to_str() const;

	private:
		void init(const Type& other);
	};

	struct Symbol
	{
	public:
		enum Kind
		{
			LABEL,
			VAR,
			CONST,
			TEMP,
			PARAM
		};

		union Value
		{
			uint addrval;
			int ival;
			char cval;
			float fval;
			std::vector<Symbol*>* arrval;
		};

		const std::string *id;
		Kind kind; // table entry type of this symbol
		Type *type; // type of this symbol
		Value value; // value of this symbol
		bool registered;
		location loc;


		/**
		 * @brief Creates a new symbol.
		 *
		 * Note that id and type are created elsewhere, but will be destroyed when this symbol is destroyed.
		 *
		 * @param id
		 * @param kind
		 * @param type
		 * @param args
		 */
		Symbol(const std::string*, const location&, Kind, Type* type = 0);

		explicit Symbol(const Symbol&);

		~Symbol();

		std::string to_str() const;
	};

	class SymbolTable
	{
	public:
		/**
		 * @brief Creates a new symbol table with no parent table.
		 */
		SymbolTable();

		virtual ~SymbolTable();

		void end_var_section();

		/**
		 * @brief Inserts or updates a symbol in the table.
		 *
		 * If the old symbol is different than the new symbol, the old symbol is deleted.
		 * This symbol will be marked as registered.
		 */
		void put(Symbol*);

		/**
		 * @brief Looks up for a symbol in the table.
		 *
		 * @param id The symbol unique id.
		 *
		 * @return A pointer to the symbol or null, if it's not found.
		 */
		const Symbol* get(const std::string& id) const;

		uint get_addr(const std::string& id) const;

		const Symbol* get(uint) const;

		uint next_addr() const;

		void show() const;

		static std::string* unique_id(float);

		static std::string* unique_id(int);


	private:
		struct AddressedElement
		{
		public:
			const Symbol *symbol;
			uint base;
			size_t size;

			AddressedElement(const Symbol*, uint, size_t);

			uint next_addr() const;
		};

		typedef std::map<std::string, uint> map_t;
		typedef std::pair<std::string, uint> pair_t;
		typedef std::vector<AddressedElement> addr_list_t;
		int m_last_var_index;

		map_t m_table;
		addr_list_t m_list;

		SymbolTable(const SymbolTable&);
	};
}

#endif /* TABLE_HPP_ */
