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
 * @file table.hpp
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */

#ifndef TABLE_HPP_
#define TABLE_HPP_ 1

#include <map>

#include "memmngr.hpp"


namespace tac
{
	class SymbolTable
	{
	public:
		/**
		 * @brief Creates a new symbol table.
		 *
		 * @param max_size the maximum number of symbols this table can hold.
		 */
		SymbolTable(uint max_size);

		~SymbolTable();

		/**
		 * @brief Inserts or updates a symbol in the table.
		 *
		 * If the old symbol is different than the new symbol, the old symbol is deleted.
		 * This symbol will be marked as registered.
		 *
		 * @param s the symbol to be inserted.
		 *
		 * @return true, if the symbol was successfully inserted; false otherwise.
		 */
		bool put(Symbol *s);

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

		uint upper_bound() const;

		void show() const;

		static std::string* unique_id(float);

		static std::string* unique_id(int);


	private:
		typedef std::map<std::string, uint> map_t;
		typedef std::map<std::string, const Symbol*> label_map_t;

		map_t m_table;
		label_map_t m_labels;
		MemoryManager m_memmngr;

		SymbolTable(const SymbolTable&);
	};
}

#endif /* TABLE_HPP_ */
