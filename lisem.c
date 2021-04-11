// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2021 Amol Surati

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#include "err.h"
#include "instr.h"

static
void li_reduce_alu_nop(struct op *op)
{
	int i;

	op->type = OP_TYPE_ADD;
	op->code = 0;
	op->cond_code = CC_NEVER;

	for (i = 0; i < 2; ++i) {
		op->src[i].type = ST_REG;
		op->src[i].u.reg.rf = RF_R;
		op->src[i].u.reg.num = 0;
	}
}

static
int li_parse_instr(struct instr *instr, const char *buf)
{
	int err, i, imm32;
	struct op *op;
	static char token[512];

	op = instr->op;

	if (op[0].code == OP_LIS)
		instr->unpack = 1;
	else if (op[0].code == OP_LIU)
		instr->unpack = 3;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	// Single source, with type imm32.
	op[0].src[0].type = ST_IMM32;

	// If the next token is a ., then we have .cc.cc
	if (!strcmp(token, ".")) {
		err = get_next_token(instr, token, buf);
		if (err) return err;
		err = parse_cond_code(token, &op[0].cond_code);
		if (err) return err;

		err = get_next_token(instr, token, buf);
		if (err) return err;
		if (strcmp(token, "."))
			return ERR_SYNTAX;

		err = get_next_token(instr, token, buf);
		if (err) return err;
		err = parse_cond_code(token, &op[1].cond_code);
		if (err) return err;

		err = get_next_token(instr, token, buf);
		if (err) return err;
	}

	// Parse the two dst.
	for (i = 0; i < 2; ++i) {
		// If the token isn't a , then error.
		if (strcmp(token, ","))
			return ERR_SYNTAX;

		err = get_next_token(instr, token, buf);
		if (err) return err;

		err = parse_dst_reg(token, &op[i].dst);
		if (err) return err;

		err = get_next_token(instr, token, buf);
		if (err) return err;

		// If the cond code is CC_NEVER, change the dst.
		// If the dst is a nop, change the cond_code to CC_NEVER.

		if  (op[i].cond_code == CC_NEVER) {
			op[i].dst.rf = RF_AB;
			op[i].dst.num = 39;
		} else if (op[i].dst.num == 39) {
			assert(op[i].dst.rf == RF_AB);
			op[i].cond_code = CC_NEVER;
		}
	}

	if (strcmp(token, ","))
		return ERR_SYNTAX;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	// Parse the imm32 source.
	err = parse_imm32(token, &imm32);
	if (err) return err;

	op[0].src[0].u.imm32 = imm32;
	return ERR_SUCCESS;
}

int lisem_resolve_instr(struct instr *instr)
{
	int err;
	struct op *op = instr->op;

	if (op[0].type == OP_TYPE_SEM)
		return ERR_SUCCESS;

	err = resolve_dst_regfiles(instr);
	if (err) return err;

	if (op[0].cond_code == CC_DEFAULT)
		op[0].cond_code = CC_ALWAYS;
	if (op[1].cond_code == CC_DEFAULT)
		op[1].cond_code = CC_ALWAYS;

	if (op[0].cond_code == CC_NEVER)
		op[0].dst.num = 0;
	if (op[1].cond_code == CC_NEVER)
		op[1].dst.num = 0;

	// If both cc are CC_NEVER, change the instruction into a nop.
	if (op[0].cond_code == CC_NEVER && op[1].cond_code == CC_NEVER) {
		li_reduce_alu_nop(&op[0]);
		li_reduce_alu_nop(&op[1]);

		instr->op[0].dst.rf = RF_A;
		instr->op[1].dst.rf = RF_B;
		instr->signal = 1;
		instr->write_swap = 0;
	}
	return ERR_SUCCESS;
}

static
int sem_parse_instr(struct instr *instr, const char *buf)
{
	int err, imm32;
	struct op *op;
	static char token[512];

	instr->unpack = 4;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	// If the token isn't a , then error.
	if (strcmp(token, ","))
		return ERR_SYNTAX;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	// Parse the imm32 source.
	err = parse_imm32(token, &imm32);
	if (err) return err;

	op = &instr->op[0];
	op->src[0].type = ST_IMM32;
	op->src[0].u.imm32 = imm32;
	op->cond_code = CC_ALWAYS;
	op->dst.rf = RF_A;
	op->dst.num = 39;

	op = &instr->op[1];
	op->cond_code = CC_NEVER;
	op->dst.rf = RF_B;
	op->dst.num = 0;

	instr->write_swap = 0;
	return ERR_SUCCESS;
}

int lisem_parse_instr(struct instr *instr, const char *buf)
{
	struct op *op;

	instr->signal = 14;

	op = &instr->op[0];
	if (op->type == OP_TYPE_LI)
		return li_parse_instr(instr, buf);
	else
		return sem_parse_instr(instr, buf);
}

void lisem_gen_code(struct instr *instr)
{
	unsigned int v;
	const struct op *op;

	// enc[0] = low 32bits, enc[1] = high 32-bits.

	op = &instr->op[0];

	v = 0;
	v |= bits_set(SIG, instr->signal);
	v |= bits_set(UNPACK, instr->unpack);
	v |= bits_set(ADD_COND, instr->op[0].cond_code);
	v |= bits_set(MUL_COND, instr->op[1].cond_code);
	if (instr->set_flag)
		v |= bits_on(SF);
	if (instr->write_swap)
		v |= bits_on(WS);
	v |= bits_set(ADD_DST, instr->op[0].dst.num);
	v |= bits_set(MUL_DST, instr->op[1].dst.num);
	instr->encoding[1] = v;

	assert(op->src[0].type == ST_IMM32);

	v = op->src[0].u.imm32;
	if (op->type == OP_TYPE_SEM) {
		v &= 0xf;
		if (op->code == OP_SEMDN)
			v |= 1u << 4;
	}
	instr->encoding[0] = v;
	return;
}
