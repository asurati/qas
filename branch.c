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

int branch_parse_instr(struct instr *instr, const char *buf)
{
	int err, i, imm32;
	static char token[512];
	struct src *src;
	char *label;
	int code, is_all;

	// Default for branches is unconditional. Note that branches
	// do not have a never-branch condition. All/Any doesn't apply
	// to unconditional branches.

	instr->signal = 15;
	is_all = -1;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	// If the next token is a ., then we have .all or .any
	if (!strcmp(token, ".")) {
		err = get_next_token(instr, token, buf);
		if (err) return err;

		if (!strcmp(token, "all"))
			is_all = 1;
		else if (!strcmp(token, "any"))
			is_all = 0;
		else
			return ERR_SYNTAX;

		err = get_next_token(instr, token, buf);
		if (err) return err;
	}

	instr->op[0].cond_code = 15;	// Always.

	// If it is a conditional branch.
	code = instr->op[0].code;
	if (code != OP_B) {
		i = 0;
		// [0] determines set/clear.
		// [1] determines all/any.
		// [3:2] determines z/n/c

		if (!is_all)
			i |= 1 << 1;

		if (code == OP_BNZ || code == OP_BNN || code == OP_BNC)
			i |= 1 << 0;

		if (code == OP_BN || code == OP_BNN)
			i |= 1 << 2;
		else if (code == OP_BC || code == OP_BNC)
			i |= 2 << 2;

		instr->op[0].cond_code = i;
	}

	// Parse the two dst.
	for (i = 0; i < 2; ++i) {
		// If the token isn't a , then error.
		if (strcmp(token, ","))
			return ERR_SYNTAX;

		err = get_next_token(instr, token, buf);
		if (err) return err;

		err = parse_dst_reg(token, &instr->op[i].dst);
		if (err) return err;

		err = get_next_token(instr, token, buf);
		if (err) return err;
	}

	if (strcmp(token, ","))
		return ERR_SYNTAX;

	err = get_next_token(instr, token, buf);
	if (err) return err;

	// There are two types of branch instructions:
	// If the token is a label, this is a relative, no-reg branch.
	// If the token is a raddr_a[0-31], this is a non-rel, reg branch.

	for (i = 0; i < 32; ++i)
		if (!strcmp(g_src_regs_a[i].name, token))
			break;

	if (i < 32) {
		instr->branch_rel = 0;
		instr->branch_reg = 1;

		// rel == 0, reg == 1
		src = &instr->op[0].src[0];
		src->type = ST_REG;
		src->u.reg.rf = RF_A;
		src->u.reg.num = g_src_regs_a[i].num;

		err = get_next_token(instr, token, buf);
		if (err) return err;

		if (strcmp(token, ","))
			return ERR_SYNTAX;

		err = get_next_token(instr, token, buf);
		if (err) return err;

		// Parse the imm32 source.
		err = parse_imm32(token, &imm32);
		if (err) return err;

		src = &instr->op[0].src[1];
		src->type = ST_IMM32;
		src->u.imm32 = imm32;
	} else {
		// token is a label

		// rel == 1, reg == 0
		instr->branch_rel = 1;
		instr->branch_reg = 0;
		src = &instr->op[0].src[0];
		src->type = ST_LABEL;
		label = malloc(strlen(token) + 1);
		assert(label);
		strcpy(label, token);
		src->u.label = label;
	}

	err = get_next_token(instr, token, buf);
	if (err) return err;

	if (strcmp(token, ";"))
		return ERR_SYNTAX;
	return ERR_SUCCESS;
}

int branch_resolve_instr(struct instr *instrs, int num_instrs,
			 struct instr *instr)
{
	int i, j, err;
	struct instr *ti;
	const char *label;
	struct op *op;

	op = &instr->op[0];

	err = resolve_dst_regfiles(instr);
	if (err) return err;

	// If it isn't a relative branch, no need to process.
	if (op->src[0].type != ST_LABEL)
		return ERR_SUCCESS;

	label = op->src[0].u.label;
	assert(label);

	ti = NULL;
	for (i = 0; i < num_instrs; ++i) {
		ti = &instrs[i];
		for (j = 0; j < ti->num_labels; ++j)
			if (!strcmp(label, ti->labels[j]))
				break;
		if (j < ti->num_labels)
			break;
	}
	if (i == num_instrs || ti == NULL)
		return ERR_SYNTAX;

	op->src[0].u.label = NULL;
	free((char *)label);

	// Convert from label to imm32.
	op->src[0].type = ST_IMM32;
	op->src[0].u.imm32 = ti->pc - (instr->pc + 4 * 8);
	return ERR_SUCCESS;
}

void branch_gen_code(struct instr *instr)
{
	unsigned int v;
	struct op *op;

	v = 0;
	op = &instr->op[0];

	v |= bits_set(SIG, instr->signal);
	v |= bits_set(PACK, op->cond_code);
	v |= bits_set(ADD_DST, instr->op[0].dst.num);
	v |= bits_set(MUL_DST, instr->op[1].dst.num);
	if (instr->write_swap)
		v |= bits_on(WS);

	if (instr->branch_rel) {
		v |= bits_on(BR_REL);
		assert(op->src[0].type == ST_IMM32);
		instr->encoding[0] = op->src[0].u.imm32;
	} else if (instr->branch_reg) {
		v |= bits_on(BR_REG);

		assert(op->src[0].type == ST_REG);
		assert(op->src[0].u.reg.rf == RF_A);

		v |= bits_set(BR_RADDR_A, op->src[0].u.reg.num);
		assert(op->src[1].type == ST_IMM32);
		instr->encoding[0] = op->src[1].u.imm32;
	} else {
		assert(0);
	}
	instr->encoding[1] = v;
}
