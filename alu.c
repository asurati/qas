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
int alu_parse_src(const char *str, struct src *out_src)
{
	size_t len, i, j;
	const struct reg_info *ri;
	static const struct reg_info *ris[] = {
		g_src_regs_a,
		g_src_regs_b,
		g_src_regs_ab,
		g_src_regs_r,
	};
	static const size_t ri_lens[] = {
		ARRAY_SIZE(g_src_regs_a),
		ARRAY_SIZE(g_src_regs_b),
		ARRAY_SIZE(g_src_regs_ab),
		ARRAY_SIZE(g_src_regs_r),
	};

	// See if it is one of the registers.
	for (i = 0; i < 4; ++i) {
		ri = ris[i];
		len = ri_lens[i];
		for (j = 0; j < len; ++j)
			if (!strcmp(ri[j].name, str))
				break;

		if (j == len)
			continue;

		out_src->type = ST_REG;
		out_src->u.reg.rf = (enum reg_file)i;
		out_src->u.reg.num = ri[j].num;
		return ERR_SUCCESS;
	}

	// If it is an small imm.
	for (i = 0; i < ARRAY_SIZE(g_src_regs_simm); ++i) {
		if (strcmp(g_src_regs_simm[i].name, str))
			continue;
		out_src->type = ST_REG;
		out_src->u.reg.rf = RF_SIMM;
		out_src->u.reg.num = g_src_regs_simm[i].num;
		return ERR_SUCCESS;
	}
	return ERR_SYNTAX;
}

static
void alu_op_nop(struct op *op)
{
	int i;

	op->code = 0;	// ALU op NOP
	op->cond_code = CC_NEVER;	// Never execute.

	// Although this NOP will change the dst.num to 0, it must not be
	// done here since this call is before the dst registers are
	// resolved.
	op->dst.rf = RF_AB;
	op->dst.num = 39;

	for (i = 0; i < 2; ++i) {
		op->src[i].type = ST_REG;
		op->src[i].u.reg.rf = RF_R;
		op->src[i].u.reg.num = 0;
	}
}

static
int alu_parse_op_add_mul(struct instr *instr, const char *buf, struct op *op)
{
	int err;
	static char token[512];
	enum cond_code cc;

	assert(op->cond_code == CC_DEFAULT);

	// Skip the opcode.
	err = get_next_token(instr, token, buf);
	if (err) return err;

	// If the next token is a ., then we have .cc
	if (!strcmp(token, ".")) {
		err = get_next_token(instr, token, buf);
		if (err) return err;

		err = parse_cond_code(token, &cc);
		if (err) return err;
		op->cond_code = cc;

		err = get_next_token(instr, token, buf);
		if (err) return err;
	}

	// If the token isn't a , then error.
	if (strcmp(token, ","))
		return ERR_SYNTAX;

	// Parse the dst.
	err = get_next_token(instr, token, buf);
	if (err) return err;
	err = parse_dst_reg(token, &op->dst);
	if (err) return err;

	err = get_next_token(instr, token, buf);
	if (err) return err;
	if (strcmp(token, ","))
		return ERR_SYNTAX;

	// Parse src0
	err = get_next_token(instr, token, buf);
	if (err) return err;
	err = alu_parse_src(token, &op->src[0]);
	if (err) return err;

	err = get_next_token(instr, token, buf);
	if (err) return err;
	if (strcmp(token, ","))
		return ERR_SYNTAX;

	// Parse src1
	err = get_next_token(instr, token, buf);
	if (err) return err;
	err = alu_parse_src(token, &op->src[1]);
	if (err) return err;
	return ERR_SUCCESS;
}

static
int alu_resolve_src_regfiles(struct instr *instr)
{
	int na, nb, n[5], i;
	enum reg_file rf;
	uint64_t mask, masks[5];
	struct src *src[4];

	src[0] = &instr->op[0].src[0];
	src[1] = &instr->op[0].src[1];
	src[2] = &instr->op[1].src[0];
	src[3] = &instr->op[1].src[1];

	memset(masks, 0, sizeof(masks));

	for (i = 0; i < 4; ++i) {
		assert(src[i]->type == ST_REG);
		if (src[i]->type != ST_REG)
			continue;

		rf = src[i]->u.reg.rf;

		// Nothing to resolve for accumulators.
		if (rf == RF_R)
			continue;

		mask = 1ull << src[i]->u.reg.num;
		masks[rf] |= mask;
	}

	for (i = 0; i < 5; ++i) {
		n[i] = 0;
		mask = masks[i];
		while (mask) {
			if (mask & 1)
				++n[i];
			mask >>= 1;
		}
	}

	// There can't be more than one simm.
	if (n[RF_SIMM] >= 2)
		return ERR_SYNTAX;

	if (n[RF_SIMM]) {
		if (instr->signal != 1)
			return ERR_SYNTAX;
		instr->signal = 13;	// Small Immediate signal.
	}

	// No common among a,ab and among b,ab.
	assert((masks[RF_A] & masks[RF_AB]) == 0);
	assert((masks[RF_B] & masks[RF_AB]) == 0);

	na = n[RF_A];
	nb = n[RF_B];

	// Single a port and single b port only.
	if (na >= 2 || nb >= 2)
		return ERR_SYNTAX;

	// No RF_AB registers to resolve.
	if (masks[RF_AB] == 0)
		return ERR_SUCCESS;

	// Since both a and b are full, any from nab can't be fulfilled.
	if (na && nb)
		return ERR_SYNTAX;

	// At least one of na or nb is empty, and there's nab to fulfill.
	for (i = 0; i < 4; ++i) {
		if (src[i]->type != ST_REG)
			continue;

		rf = src[i]->u.reg.rf;
		if (rf != RF_AB)
			continue;

		mask = 1ull << src[i]->u.reg.num;

		if (masks[RF_A] == 0 || masks[RF_A] == mask)
			rf = RF_A;
		else if (masks[RF_B] == 0 || masks[RF_B] == mask)
			rf = RF_B;
		else
			return ERR_SYNTAX;
		masks[rf] = mask;
		src[i]->u.reg.rf = rf;
	}
	return ERR_SUCCESS;
}

static
int alu_reduce_op(struct op *op)
{
	int n_nops, n_ios, i;
	struct src *src;

	// If all sources are nop register, turn op to a nop.
	// If dst is a nop, and no src is an IO register, turn op to a nop.
	// If cc is never, turn op to a nop.

	n_nops = n_ios = 0;
	for (i = 0; i < 2; ++i) {
		src = &op->src[i];
		if (src->type != ST_REG)
			continue;
		if (src->u.reg.rf != RF_A && src->u.reg.rf != RF_B)
			continue;

		if (src->u.reg.num == 39) {
			// Reading a nop doesn't give a definite value.
			// Change to r0
			src->u.reg.rf = RF_R;
			src->u.reg.num = 0;
			++n_nops;
		} else if (src->u.reg.num >= 32) {
			++n_ios;
		}
	}

	// If both the srcs are NOPs, change the op to NOP.
	if (n_nops == 2)
		op->cond_code = CC_NEVER;

	// If the dst is a NOP, and no src is an IO register, change the op to
	// NOP.
	if (op->dst.num == 39 && n_ios == 0)
		op->cond_code = CC_NEVER;

	// Change to NOP, as necessary; or, set the cond_code.
	if (op->cond_code == CC_NEVER)
		alu_op_nop(op);
	else if (op->cond_code == CC_DEFAULT)
		op->cond_code = CC_ALWAYS;
	return ERR_SUCCESS;
}

int alu_parse_instr(struct instr *instr, const char *buf)
{
	int err;
	static char token[512];
	struct op *op;

	err = ERR_SUCCESS;
	op = instr->op;
	if (op[0].code == 0)
		alu_op_nop(&op[0]);
	else
		err = alu_parse_op_add_mul(instr, buf, &op[0]);
	if (err) return err;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	err = parse_op_code(token, &op[1]);
	if (err) return err;

	if (op[1].code == 0)
		alu_op_nop(&op[1]);
	else
		err = alu_parse_op_add_mul(instr, buf, &op[1]);
	if (err) return err;
	return ERR_SUCCESS;
}

int alu_resolve_instr(struct instr *instr)
{
	int err;
	struct op *op = instr->op;

	err = alu_resolve_src_regfiles(instr);
	if (err) return err;

	err = alu_reduce_op(&op[0]);
	if (err) return err;
	err = alu_reduce_op(&op[1]);
	if (err) return err;

	err = resolve_dst_regfiles(instr);
	if (err) return err;

	if (op[0].cond_code == CC_NEVER)
		op[0].dst.num = 0;
	if (op[1].cond_code == CC_NEVER)
		op[1].dst.num = 0;
	return ERR_SUCCESS;
}

void alu_gen_code(struct instr *instr)
{
	unsigned int v;
	enum reg_file rf;
	int num;
	int raddr_a, raddr_b;
	struct src *src[4];

	// enc[0] = low 32bits, enc[1] = high 32-bits.

	v = 0;
	v |= bits_set(ADD_DST, instr->op[0].dst.num);
	v |= bits_set(MUL_DST, instr->op[1].dst.num);

	if (instr->write_swap) v |= bits_on(WS);
	if (instr->set_flag) v |= bits_on(SF);

	assert(instr->op[0].cond_code != CC_DEFAULT);
	assert(instr->op[1].cond_code != CC_DEFAULT);

	v |= bits_set(ADD_COND, instr->op[0].cond_code);
	v |= bits_set(MUL_COND, instr->op[1].cond_code);

	v |= bits_set(SIG, instr->signal);
	instr->encoding[1] = v;

	v = 0;

	// Fill in the 4 input slots.
	raddr_a = raddr_b = 0;

#define FILL_SLOT(slot)							\
	do {								\
		if (rf == RF_R) {					\
			v |= bits_set(slot, num);			\
		} else if (rf == RF_A) {				\
			v |= bits_set(slot, 6);				\
			raddr_a = num;					\
		} else if (rf == RF_B || rf == RF_SIMM)	{		\
			v |= bits_set(slot, 7);				\
			raddr_b = num;					\
		} else {						\
			assert(0);					\
		}							\
	} while (0)

	src[0] = &instr->op[0].src[0];
	src[1] = &instr->op[0].src[1];
	src[2] = &instr->op[1].src[0];
	src[3] = &instr->op[1].src[1];

	assert(src[0]->type == ST_REG);
	assert(src[1]->type == ST_REG);
	assert(src[2]->type == ST_REG);
	assert(src[3]->type == ST_REG);

	// add_src0.
	rf  = src[0]->u.reg.rf;
	num = src[0]->u.reg.num;
	FILL_SLOT(ADD_A);

	// add_src1.
	rf  = src[1]->u.reg.rf;
	num = src[1]->u.reg.num;
	FILL_SLOT(ADD_B);

	// mul_src0.
	rf  = src[2]->u.reg.rf;
	num = src[2]->u.reg.num;
	FILL_SLOT(MUL_A);

	// mul_src1.
	rf  = src[3]->u.reg.rf;
	num = src[3]->u.reg.num;
	FILL_SLOT(MUL_B);

	v |= bits_set(RADDR_A, raddr_a);
	v |= bits_set(RADDR_B, raddr_b);

	// Fill in the opcodes.
	v |= bits_set(MUL_OP, instr->op[1].code);
	v |= bits_set(ADD_OP, instr->op[0].code);

	instr->encoding[0] = v;
}
