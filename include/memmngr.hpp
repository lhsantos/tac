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
 * @file memmngr.hpp
 *
 * @brief
 *
 * @date 2014-11-19
 *
 * @author Luciano Santos
 */

#ifndef INCLUDE_MEMMNGR_HPP_
#define INCLUDE_MEMMNGR_HPP_

#include <stdint.h>
#include <vector>
#include <list>

#include "symbol.hpp"


namespace tac
{
	class SymbolTable;

	/**
	 * @brief A log(n) access time memory manager.
	 */
	class MemoryManager
	{
	public:
		/**
		 * @brief Creates a memory manager with given maximum size (number of symbols).
		 *
		 * @param max_size the maximum size of the memory.
		 */
		MemoryManager(uint max_size);

		/**
		 * Releases all (system) memory allocated by this manager.
		 */
		~MemoryManager();

		/**
		 * Allocates a new block of memory of given size (number of symbols), if possible.
		 *
		 * @param size the size of the block.
		 * @param addr if a new block was successfully allocated, receives the symbolic address
		 * of the block; remains unchanged otherwise.
		 *
		 * @return true, if the block was successfully allocated; false otherwise.
		 */
		bool alloc(size_t size, uint& addr);

		/**
		 * Inserts the given symbol (possibly an array) in a free block of memory and
		 * associates the block with the given symbol.
		 *
		 * @param s the symbol to be inserted.
		 * @param addr if successful, receives the symbolic address of the block; remains
		 * unchanged otherwise.
		 *
		 * @return true, if successful; false otherwise.
		 */
		bool put(const Symbol *s, uint& addr);

		/**
		 * Retrieves the symbol at a specified address.
		 *
		 * @param addr the address to be accessed.
		 *
		 * @return the symbol, if the address is valid; null, otherwise.
		 */
		const Symbol* get(uint addr) const;

		/**
		 * Retrieves the memory block at a specified address, as a symbol.
		 *
		 * @param addr the address to be accessed.
		 *
		 * @return the symbol, if the address is valid; null, otherwise.
		 */
		const Symbol* get_block(uint addr) const;

		/**
		 * Releases the memory block that starts in given address.
		 *
		 * @param addr the address.
		 *
		 * @return true if successfully releases the block; false otherwise (address not found).
		 */
		bool free(uint addr);

		/**
		 * Releases all the blocks allocated by this manager.
		 */
		void clear();

		/**
		 * How much space (number of symbols) is available in the memory?
		 *
		 * @return the amount of space.
		 */
		size_t available() const;

		/**
		 * Retrieves the upper bound of the block with largest address.
		 *
		 * @return upper address bound or 0, if no block is allocated yet.
		 */
		uint upper_bound() const;

	private:
		friend class SymbolTable;

		struct Block
		{
		public:
			uint base;
			size_t size;

			uint upper_bound() const;

			bool operator<(const Block&) const;

			bool operator<(uint) const;
		};

		struct MemoryBlock : public Block
		{
		public:
			const Symbol *symbol;

			MemoryBlock(uint, size_t, const Symbol*);
		};

		typedef std::vector<MemoryBlock> vector_t;
		typedef std::list<Block> stall_list_t;


		const uint m_max_size;

		vector_t m_blocks;
		uint m_size;
		stall_list_t m_stalls;

		bool put(const Symbol*, size_t, uint&);
	};
}

#endif /* INCLUDE_MEMMNGR_HPP_ */
