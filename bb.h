#ifndef BB_H
#define BB_H

#include "ir.h"

/**
 * @brief a basic block
 */
typedef struct {
    stmt_t begin;
    stmt_t end;
} bbtable_t;

extern bbtable_t bbtab[]; ///< basic block table

extern int bbtabSize; ///< number of basic blocks

/**
 * @brief build basic blocks for each function
 */
void buildBasicBlocks();

/**
 * @brief dump a single basic block to stream
 * @param fd output stream
 * @param b basic block index
 */
void dumpBB(FILE *fd, bb_t b);

/**
 * @brief dump basic blocks to stream
 * @param fd output stream
 */
void dumpBasicBlocks(FILE *fd);

#endif
