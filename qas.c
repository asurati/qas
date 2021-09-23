// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2021 Amol Surati

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "qas.h"

// 0                    1                      2
// add[i].cc dst,src0,src1 mul[i].cc,dst,src0,src1 signal,unpack,pack,pm,sf
// add[i].cc dst,src0,src1 signal,unpack,pack,pm,sf (mul nop)
// mul[i].cc dst,src0,src1 signal,unpack,pack,pm,sf (add nop)
// signal,unpack,pack,pm,sf (add and mul nop)
//
// li.acc.mcc adst,mdst,imm pack,pm,sf
// lis.acc.mcc adst,mdst,imm pack,pm,sf
// liu.acc.mcc adst,mdst,imm pack,pm,sf
// semup sem
// semdn sem
//
// b.cc raddr_a, label
// bl.cc adst,mdst,raddr_a,label
// Simplified branches:
// b.cc label
// bl.cc adst,label

static int token_start[MAX_TOKENS];
static int token_end[MAX_TOKENS];
static int num_tokens;

static
void get_token(struct instr *in, char *out)
{
	int ts, te, t, len;

	t = in->curr_token++;

	assert(t < num_tokens);

	ts = token_start[t];
	te = token_end[t];

	len = te - ts;
	assert(len < 128);

	if (len > 127)
		len = 127;

	memcpy(out, &in->buf[ts], len);
	out[len] = 0;
}

static
void parse_nop(struct instr *in)
{
	struct op *op;

	in->sig = OP_SIG_NONE;

	op = &in->op;
	op->code[0] = op->code[1] = OP_NOP;
	op->cc[0] = op->cc[1] = CC_NEVER;
	op->dst[0].rf = op->dst[1].rf = RF_AB;
	op->dst[0].num = op->dst[1].num = 39;
	op->src[0].rf = op->src[1].rf = op->src[2].rf = op->src[3].rf = RF_ACC;
	op->src[0].num = op->src[1].num = op->src[2].num = op->src[3].num = 0;
}

static
int parse_cc(const char *str, enum cc *out)
{
	int i, num;

	num = NUM_ARR(g_cc_info);
	for (i = 0; i < num; ++i) {
		if (!strcmp(g_cc_info[i].name, str))
			break;
	}

	if (i == num)
		return -EINVAL;
	*out = g_cc_info[i].code;
	return ESUCC;
}

static
int parse_op_code(const char *str, enum op_code *out)
{
	int i, num;

	num = NUM_ARR(g_op_info);
	for (i = 0; i < num; ++i) {
		if (!strcmp(g_op_info[i].name, str))
			break;
	}
	assert(i < num);
	if (i == num)
		return -EINVAL;
	*out = g_op_info[i].code;
	return ESUCC;
}

static
int parse_reg(const char *str, char is_src, struct reg *out)
{
	int num, i;
	const struct reg_info *ri;

	if (is_src) {
		ri = g_src_reg_info;
		num = NUM_ARR(g_src_reg_info);
	} else {
		ri = g_dst_reg_info;
		num = NUM_ARR(g_dst_reg_info);
	}

	for (i = 0; i < num; ++i) {
		if (!strcmp(ri[i].name, str))
			break;
	}

	if (i == num)
		return -EINVAL;

	out->rf = ri[i].rf;
	out->num = ri[i].num;
	return ESUCC;
}

static inline
int parse_src_reg(const char *str, struct reg *out)
{
	return parse_reg(str, 1, out);
}

static inline
int parse_dst_reg(const char *str, struct reg *out)
{
	return parse_reg(str, 0, out);
}

static
int parse_num(const char *str, char is_hex, int *out)
{
	int num, len, i;

	len = strlen(str);
	num = 0;
	if (!is_hex) {
		for (i = 0; i < len; ++i) {
			if (!isdigit(str[i]))
				return -EINVAL;
			num *= 10;
			num += str[i] - '0';
		}
	} else {
		for (i = 0; i < len; ++i) {
			if (!is_hex_digit(str[i]))
				return -EINVAL;
			num <<= 4;
			if (str[i] >= 'a' && str[i] <= 'f')
				num += 10 + str[i] - 'a';
			else
				num += str[i] - '0';
		}
	}
	*out = num;
	return ESUCC;
}

static
int parse_src_imm(const char *str, struct reg *out)
{
	char is_hex;
	int num, err;

	if (!isdigit(str[0]))
		return -EINVAL;

	is_hex = 0;
	if (!strncmp(str, "0x", 2))
		is_hex = 1;

	if (is_hex)
		err = parse_num(&str[2], is_hex, &num);
	else
		err = parse_num(str, is_hex, &num);
	if (err)
		return err;

	out->rf = RF_IMM;
	out->num = num;
	return ESUCC;
}

static
int parse_op_signals(struct instr *in, enum op_code code)
{
	// Can't have more than one signals.
	if (in->sig != OP_SIG_NONE)
		return -EINVAL;
	in->sig = code;
	return ESUCC;
}

static
int parse_op_flags(struct instr *in, enum op_code code)
{
	if (code == OP_FLAGS_SF)
		in->sf = 1;
	else if (code == OP_FLAGS_PM)
		in->pm = 1;
	else
		return -EINVAL;
	return ESUCC;
}

static
void print_tokens(const char *buf)
{
	int i, ts, te, j;

	for (i = 0; i < num_tokens; ++i) {
		ts = token_start[i];
		te = token_end[i];
		printf("\'");
		for (j = ts; j < te; ++j)
			printf("%c", buf[j]);
		printf ("\'");
		if (i != num_tokens - 1)
			printf("    ");
	}
	printf("\n");
}

static
int parse_op_load_imm(struct instr *in, int code)
{
	int err;
	struct op *op;
	static char token[128];

	in->sig = OP_SIG_LI;

	op = &in->op;
	op->cc[0] = op->cc[1] = CC_ALWAYS;
	op->code[0] = code;

	switch (op->code[0]) {
	case OP_IMM_LI:
		in->unpack = 0;
		break;
	case OP_IMM_LIS:
		in->unpack = 1;
		break;
	case OP_IMM_LIU:
		in->unpack = 3;
		break;
	case OP_SEM_SEMUP:
	case OP_SEM_SEMDN:
		in->unpack = 4;
		break;
	default:
		return -EINVAL;
	}

	// A condition code follows.
	get_token(in, token);
	err = parse_cc(token, &op->cc[0]);
	if (!err) {
		// If there was one cc, another should follow too.
		get_token(in, token);
		err = parse_cc(token, &op->cc[1]);
		if (err)
			return err;
		get_token(in, token);
	}

	// A dst register follows
	err = parse_dst_reg(token, &op->dst[0]);
	if (err)
		return err;

	// A dst register follows
	get_token(in, token);
	err = parse_dst_reg(token, &op->dst[1]);
	if (err)
		return err;

	// An immediate (not small immediate) src follows
	get_token(in, token);
	return parse_src_imm(token, &op->src[0]);
}

static
int parse_op_branch(struct instr *in, enum op_code code)
{
	int err;
	struct op *op;
	static char token[128];

	in->sig = OP_SIG_BR;

	op = &in->op;
	op->code[0] = code;
	op->cc[0] = CC_ALWAYS;

	// A condition code follows.
	get_token(in, token);
	err = parse_cc(token, &op->cc[0]);
	if (!err)
		get_token(in, token);

	// Branch with Link needs a dst register to save the return address.
	if (op->code[0] == OP_BR_BL) {
		// A dst register follows
		err = parse_dst_reg(token, &op->dst[0]);
		if (err)
			return err;
		get_token(in, token);
	}

	// A src label follows, or a RF_A register [0-31] follows.
	err = parse_src_reg(token, &op->src[0]);
	if (err) {
		// Perhaps a label?
		op->src_label = malloc(strlen(token) + 1);
		strcpy(op->src_label, token);
	}
	return ESUCC;
}

static
int parse_op_add_mul(struct instr *in, enum op_code code, int op_ix)
{
	int err;
	static char token[128];
	struct op *op;
	enum cc cc;

	op = &in->op;

	op->code[op_ix] = code;
	op->cc[op_ix] = CC_ALWAYS;

	// Does a condition code follow?
	get_token(in, token);
	err = parse_cc(token, &cc);
	if (!err) {
		// Parsed a condition code.
		op->cc[op_ix] = cc;
		get_token(in, token);
	}

	// A dst register follows
	err = parse_dst_reg(token, &op->dst[op_ix]);
	if (err)
		return err;

	// A src reg follows
	get_token(in, token);
	err = parse_src_reg(token, &op->src[op_ix * 2]);
	if (err)
		return err;

	// A src reg follows
	get_token(in, token);
	err = parse_src_reg(token, &op->src[op_ix * 2 + 1]);
	return err;
}

static
int parse_op_add(struct instr *in, enum op_code code)
{
	return parse_op_add_mul(in, code, 0);
}

static
int parse_op_mul(struct instr *in, enum op_code code)
{
	return parse_op_add_mul(in, code, 1);
}

static
int parse_op_add_simm(struct instr *in, enum op_code code)
{
	in->sig = OP_SIG_SIMM;
	return parse_op_add_mul(in, code, 0);
}

static
int parse_op_mul_simm(struct instr *in, enum op_code code)
{
	in->sig = OP_SIG_SIMM;
	return parse_op_add_mul(in, code, 1);
}

static
int parse(struct instr *in)
{
	enum op_code code;
	int err, is_op_add, is_op_li;
	static char token[128];

	// Default is a NOP.
	parse_nop(in);

	get_token(in, token);
	if (!strcmp(token, ";"))
		return ESUCC;
	err = parse_op_code(token, &code);
	if (err)
		return err;

	err = -EINVAL;
	if (code >= OP_ADD_FADD && code <= OP_ADD_V8SUBS) {
		is_op_add = 1;
		err = parse_op_add(in, code);
	} else if (code >= OP_MUL_FMUL && code <= OP_MUL_V8MAX) {
		goto check_mul;
	} else if (code >= OP_ADD_FADDI && code <= OP_ADD_V8SUBSI) {
		is_op_add = 1;
		err = parse_op_add_simm(in, code);
	} else if (code >= OP_MUL_FMULI && code <= OP_MUL_V8MAXI) {
		goto check_mul;
	} else if (code >= OP_MUL_FMULROTR5 && code <= OP_MUL_FMULROT15) {
		goto check_mul;
	} else if (code >= OP_BR_B && code <= OP_BR_BL) {
		err = parse_op_branch(in, code);
	} else if ((code >= OP_IMM_LI && code <= OP_IMM_LIU) ||
		   (code >= OP_SEM_SEMUP && code <= OP_SEM_SEMDN)) {
		is_op_li = 1;
		err = parse_op_load_imm(in, code);
	} else if (code >= OP_SIG_BREAK && code <= OP_SIG_LD_ALPHA) {
		goto check_sigs;
	} else if (code >= OP_FLAGS_SF && code <= OP_FLAGS_PM) {
		goto check_flags;
	}

	if (err)
		return err;

	// Are we at the end of the instruction?
	get_token(in, token);
	if (!strcmp(token, ";"))
		return ESUCC;
	err = parse_op_code(token, &code);
	if (err)
		return err;

	if (is_op_add)
		goto check_mul;
	else if (is_op_li)
		goto check_flags;
	else
		return -EINVAL;

check_mul:
	// mul, signals, and flags
	err = -EINVAL;
	if (code >= OP_MUL_FMUL && code <= OP_MUL_V8MAX)
		err = parse_op_mul(in, code);
	else if (code >= OP_MUL_FMULI && code <= OP_MUL_V8MAXI)
		err = parse_op_mul_simm(in, code);
	else if (code >= OP_MUL_FMULROTR5 && code <= OP_MUL_FMULROT15)
		err = parse_op_mul_simm(in, code);
	else
		goto check_sigs;
	if (err)
		return err;

	get_token(in, token);
	if (!strcmp(token, ";"))
		return ESUCC;
	err = parse_op_code(token, &code);
	if (err)
		return err;
check_sigs:
	// signals, and flags.
	err = -EINVAL;
	if (code >= OP_SIG_BREAK && code <= OP_SIG_LD_ALPHA)
		err = parse_op_signals(in, code);
	else
		goto check_flags;
	if (err)
		return err;

	get_token(in, token);
	if (!strcmp(token, ";"))
		return ESUCC;
	err = parse_op_code(token, &code);
	if (err)
		return err;
check_flags:
	err = -EINVAL;
	if (code >= OP_FLAGS_SF && code <= OP_FLAGS_PM)
		err = parse_op_flags(in, code);
	if (err)
		return err;

	get_token(in, token);
	if (!strcmp(token, ";"))
		return ESUCC;

	// Nothing should be there after flags.
	return -EINVAL;
}

static
void resolve_dst_regs(struct instr *in)
{
	struct op *op;

	op = &in->op;
	in->ws = 0;
	if (op->dst[0].rf == RF_B || op->dst[1].rf == RF_A)
		in->ws = 1;

	if (in->ws) {
		op->dst[0].rf = RF_B;
		op->dst[1].rf = RF_A;
	} else {
		op->dst[0].rf = RF_A;
		op->dst[1].rf = RF_B;
	}
}

static
int verify_branch(struct instr *in, const struct instr *ins, int num_instrs)
{
	struct op *op;
	int i, j;
	const struct instr *t;

	op = &in->op;
	resolve_dst_regs(in);

	if (op->src_label == NULL) {
		// The register should be RF_A [0-31].
		if (op->src[0].rf != RF_A || op->src[0].num > 31)
			return -EINVAL;
		return ESUCC;
	}

	// Else, check if there is a target instruction.
	for (i = 0; i < num_instrs; ++i) {
		t = &ins[i];
		for (j = 0; j < t->num_labels; ++j) {
			if (!strcmp(op->src_label, t->labels[j]))
				break;
		}
		if (j < t->num_labels)
			break;
	}

	// Non-existent label.
	if (i == num_instrs)
		return -EINVAL;

	op->src[0].rf = RF_IMM;
	op->src[0].num = t->pc - (in->pc + 4 * 8);
	return ESUCC;
}

static
int verify_alu(struct instr *in)
{
	struct op *op;
	enum op_code code;
	uint64_t mask[6], t;
	int is_io_reg, i;


	resolve_dst_regs(in);

	op = &in->op;

	// ALUs do not deal with branch condition codes.
	if (op->cc[0] >= CC_ALL_Z || op->cc[1] >= CC_ALL_Z)
		return -EINVAL;

	// Behaviour is undefined if both ALUs write to the same accumulator
	// or IO register.
	if (op->dst[0].num == op->dst[1].num &&
	    op->dst[0].num > 31 && op->dst[0].num != 39)
		return -EINVAL;

	memset(mask, 0, sizeof(mask));
	for (i = 0; i < 4; ++i) {
		if (op->src[i].rf == RF_IMM)
			continue;
		mask[op->src[i].rf] |= 1ul << op->src[i].num;
	}

	code = op->code[1];

	// If mul output is to be rotated, there shouldn't be any small
	// immediates, or anyone reading from RF_B.
	if (code >= OP_MUL_FMULROTR5 && code <= OP_MUL_FMULROT15 &&
	    (mask[RF_SIMM] || mask[RF_B]))
		return -EINVAL;

	// If mul output is to be rotated, the sources must be from RF_ACC,
	// r0-r3.
	if (code >= OP_MUL_FMULROTR5 && code <= OP_MUL_FMULROT15 &&
	    (op->src[2].rf != RF_ACC || op->src[3].rf != RF_ACC))
		return -EINVAL;

	// If there are small immediate sources, they should be same.
	if (count_ones(mask[RF_SIMM]) > 1)
		return -EINVAL;

	// If there are small immediate sources, or, if mul output is to be
	// rotated, any RF_AB source must be converted to RF_A.
	if (code >= OP_MUL_FMULROTR5 && code <= OP_MUL_FMULROT15 &&
	    mask[RF_SIMM]) {
		for (i = 0; i < 4; ++i) {
			if (op->src[i].rf != RF_AB)
				continue;
			op->src[i].rf = RF_A;
			t = 1ul << op->src[i].num;
			mask[RF_A] |= t;
		}
	}

	// If there are RF_A sources, they should be same.
	// Similarly for RF_B sources.
	if (count_ones(mask[RF_A]) > 1)
		return -EINVAL;
	if (count_ones(mask[RF_B]) > 1)
		return -EINVAL;

	// If there are RF_AB sources, try to resolve them.
	for (i = 0; i < 4; ++i) {
		if (op->src[i].rf != RF_AB)
			continue;

		t = 1ul << op->src[i].num;

		// Try resolving with RF_A.
		if (mask[RF_A] & t) {
			op->src[i].rf = RF_A;
			mask[RF_A] |= t;
			continue;
		}

		// Try resolving with RF_B.
		if (mask[RF_B] & t) {
			op->src[i].rf = RF_B;
			mask[RF_B] |= t;
			continue;
		}
	}

	for (i = 0; i < 4; ++i) {
		if (op->src[i].rf != RF_AB)
			continue;

		t = 1ul << op->src[i].num;

		// Try resolving with RF_A if RF_A is unused, or same.
		if (!mask[RF_A] || (mask[RF_A] & t)) {
			op->src[i].rf = RF_A;
			mask[RF_A] |= t;
			continue;
		}

		// Try resolving with RF_B if RF_B is unused, or same,
		if (!mask[RF_B] || (mask[RF_B] & t)) {
			op->src[i].rf = RF_B;
			mask[RF_B] |= t;
			continue;
		}

		// Can't be resolved.
		return -EINVAL;
	}

	// If the dst is NOP and the sources are not IO registers, change
	// the cc to CC_NEVER.
	is_io_reg = 0;
	is_io_reg |=
		(op->src[0].rf == RF_A || op->src[0].rf == RF_B) &&
		(op->src[0].num > 31);
	is_io_reg |=
		(op->src[1].rf == RF_A || op->src[1].rf == RF_B) &&
		(op->src[1].num > 31);
	if (op->dst[0].num == 39 && !is_io_reg)
		op->cc[0] = CC_NEVER;

	is_io_reg = 0;
	is_io_reg |=
		(op->src[2].rf == RF_A || op->src[2].rf == RF_B) &&
		(op->src[2].num > 31);
	is_io_reg |=
		(op->src[3].rf == RF_A || op->src[3].rf == RF_B) &&
		(op->src[3].num > 31);
	if (op->dst[1].num == 39 && !is_io_reg)
		op->cc[1] = CC_NEVER;

	return ESUCC;
}

static
int verify_load_imm(struct instr *in)
{
	struct op *op;

	resolve_dst_regs(in);

	op = &in->op;

	// If any dst is a NOP, change cc to CC_NEVER
	if (op->dst[0].num == 39)
		op->cc[0] = CC_NEVER;
	if (op->dst[1].num == 39)
		op->cc[1] = CC_NEVER;

	// Semaphore num should be in range.
	if ((op->code[0] >= OP_SEM_SEMUP && op->code[0] <= OP_SEM_SEMDN) &&
	    (op->src[0].num < 0 || op->src[0].num > 15))
		return -EINVAL;
	return ESUCC;
}

static
int verify(struct instr *in, const struct instr *ins, int num_instrs)
{
	int err;
	enum op_code code;
	struct op *op;

	op = &in->op;
	code = op->code[0];

	err = -EINVAL;
	if ((code == OP_NOP) ||
	    (code >= OP_ADD_FADD && code <= OP_ADD_V8SUBS) ||
	    (code >= OP_ADD_FADDI && code <= OP_ADD_V8SUBSI)) {
		err = verify_alu(in);
	} else if (code >= OP_BR_B && code <= OP_BR_BL) {
		err = verify_branch(in, ins, num_instrs);
	} else if ((code >= OP_IMM_LI && code <= OP_IMM_LIU) ||
		   (code >= OP_SEM_SEMUP && code <= OP_SEM_SEMDN)) {
		err = verify_load_imm(in);
	}
	return err;
}

static
int encode_load_imm(struct instr *in)
{
	int esig, ecc[2];
	unsigned int val;
	struct op *op;

	op = &in->op;

	esig = encode_sig(in->sig);
	ecc[0] = encode_cond(op->cc[0]);
	ecc[1] = encode_cond(op->cc[1]);

	if (esig < 0 || ecc[0] < 0 || ecc[1] < 0)
		return -EINVAL;

	val = 0;
	val |= bits_set(ENC_SIG, esig);
	val |= bits_set(ENC_UNPACK, in->unpack);
	val |= bits_set(ENC_PACK, in->pack);
	val |= bits_set(ENC_COND_ADD, ecc[0]);
	val |= bits_set(ENC_COND_MUL, ecc[1]);
	val |= bits_set(ENC_WADDR_ADD, op->dst[0].num);
	val |= bits_set(ENC_WADDR_MUL, op->dst[1].num);
	if (in->pm)
		val |= bits_on(ENC_PM);
	if (in->sf)
		val |= bits_on(ENC_SF);
	if (in->ws)
		val |= bits_on(ENC_WS);
	in->hi = val;
	in->lo = op->src[0].num;
	return ESUCC;
}

static
int encode_branch(struct instr *in)
{
	struct op *op;
	int ecc, esig;
	unsigned int val;

	op = &in->op;

	esig = encode_sig(in->sig);
	ecc = encode_cond_br(op->cc[0]);

	if (esig < 0 || ecc < 0)
		return -EINVAL;

	val = 0;
	if (op->src_label == NULL) {
		// Jump to register.
		val |= bits_on(ENC_BR_REG);
		val |= bits_set(ENC_BR_RADDR_A, op->src[0].num);
	} else {
		// Jump to label.
		val |= bits_on(ENC_BR_REL);
		in->lo = op->src[0].num;
	}

	val |= bits_set(ENC_SIG, esig);
	val |= bits_set(ENC_BR_COND, ecc);
	val |= bits_set(ENC_WADDR_ADD, op->dst[0].num);
	val |= bits_set(ENC_WADDR_MUL, op->dst[1].num);
	if (in->ws)
		val |= bits_on(ENC_WS);
	in->hi = val;
	return ESUCC;
}

static
int encode_alu(struct instr *in)
{
	int esig, ecc[2], eop[2], emuxes[4];
	int raddr_a, raddr_b, val, i;
	struct op *op;
	enum op_code code;

	op = &in->op;

	esig = encode_sig(in->sig);
	ecc[0] = encode_cond(op->cc[0]);
	ecc[1] = encode_cond(op->cc[1]);
	eop[0] = encode_alu_op_add(op->code[0]);
	eop[1] = encode_alu_op_mul(op->code[1]);

	if (esig < 0 || ecc[0] < 0 || ecc[1] < 0 || eop[0] < 0 || eop[1] < 0)
		return -EINVAL;

	val = 0;
	val |= bits_set(ENC_SIG, esig);
	val |= bits_set(ENC_UNPACK, in->unpack);
	val |= bits_set(ENC_PACK, in->pack);
	val |= bits_set(ENC_COND_ADD, ecc[0]);
	val |= bits_set(ENC_COND_MUL, ecc[1]);
	val |= bits_set(ENC_WADDR_ADD, op->dst[0].num);
	val |= bits_set(ENC_WADDR_MUL, op->dst[1].num);
	if (in->pm)
		val |= bits_on(ENC_PM);
	if (in->sf)
		val |= bits_on(ENC_SF);
	if (in->ws)
		val |= bits_on(ENC_WS);
	in->hi = val;

	raddr_a = raddr_b = 39;	// NOP

	for (i = 0; i < 4; ++i) {
		switch (op->src[i].rf) {
		case RF_A:
			raddr_a = op->src[i].num;
			emuxes[i] = 6;
			break;
		case RF_B:
		case RF_SIMM:
			raddr_b = op->src[i].num;
			emuxes[i] = 7;
			break;
		case RF_ACC:
			emuxes[i] = op->src[i].num;
			break;
		default:
			assert(0);
			break;
		}
	}

	code = op->code[1];
	if (code >= OP_MUL_FMULROTR5 && code <= OP_MUL_FMULROT15)
		raddr_b = 48 + (code - OP_MUL_FMULROTR5);

	val = 0;
	val |= bits_set(ENC_ALU_OP_MUL, eop[1]);
	val |= bits_set(ENC_ALU_OP_ADD, eop[0]);
	val |= bits_set(ENC_ALU_RADDR_A, raddr_a);
	val |= bits_set(ENC_ALU_RADDR_B, raddr_b);
	val |= bits_set(ENC_ALU_ADD_0, emuxes[0]);
	val |= bits_set(ENC_ALU_ADD_1, emuxes[1]);
	val |= bits_set(ENC_ALU_MUL_0, emuxes[2]);
	val |= bits_set(ENC_ALU_MUL_1, emuxes[3]);
	in->lo = val;
	return ESUCC;
}

static
int encode(struct instr *in)
{
	int err;
	enum op_code code;
	struct op *op;

	op = &in->op;
	code = op->code[0];

	err = -EINVAL;
	if ((code == OP_NOP) ||
	    (code >= OP_ADD_FADD && code <= OP_ADD_V8SUBS) ||
	    (code >= OP_ADD_FADDI && code <= OP_ADD_V8SUBSI)) {
		err = encode_alu(in);
	} else if (code >= OP_BR_B && code <= OP_BR_BL) {
		err = encode_branch(in);
	} else if ((code >= OP_IMM_LI && code <= OP_IMM_LIU) ||
		   (code >= OP_SEM_SEMUP && code <= OP_SEM_SEMDN)) {
		err = encode_load_imm(in);
	}
	return err;
}

// [ls, le)
static
int tokenize(const char *buf, int ls, int le)
{
	char in_token, is_delim;
	int nt, i;

	in_token = num_tokens = nt = 0;
	for (i = ls; i < le; ++i) {
		// Is it a delimiter?
		is_delim = 0;
		is_delim |= isspace(buf[i]) ? 1 : 0;
		is_delim |= buf[i] == ',' ? 1 : 0;
		is_delim |= buf[i] == '.' ? 1 : 0;
		is_delim |= buf[i] == ';' ? 1 : 0;

		if (is_delim) {
			// It is a delimiter; close any previous token.
			if (in_token) {
				in_token = 0;
				token_end[nt++] = i;
			}

			if (isspace(buf[i]) || buf[i] == ',' || buf[i] == '.')
				continue;

			// Delim ; is a token.
			assert(nt < MAX_TOKENS);
			token_start[nt] = i;
			token_end[nt++] = i + 1;

			assert(buf[i] == ';');
			break;
		} else if (!in_token) {
			// Else it is a non-delim non-space char. Begin a
			// token if not already begun.
			assert(nt < MAX_TOKENS);
			token_start[nt] = i;
			in_token = 1;
		}
	}

	assert(i < le);

	// We must see a ;, otherwise invalid instruction.
	if (i == le)
		return -EINVAL;

	assert(nt <= MAX_TOKENS);
	num_tokens = nt;
	return ESUCC;
}

static
int parse_labels(struct instr *in, int ix, int size, int *out_le,
		 int *out_ls)
{
	int i, ls, j, nl;
	const char *buf;
	char **p;

	buf = in->buf;

	i = ix;
	for (;;) {
		//Skip any whitespace.
		for (i = i; i < size; ++i) {
			if (!isspace(buf[i]))
				break;
		}

		if (i == size) {
			*out_ls = -1;
			*out_le = i;
			return ESUCC;
		}

		// Scan for delims.
		ls = i;
		for (i = i; i < size; ++i) {
			if (buf[i] == ';')
				break;
			if (buf[i] == '#')
				break;
			if (buf[i] == ':')
				break;
		}

		// Could not find a ;
		if (i == size)
			return -EINVAL;

		// Found an instruction
		if (buf[i] == ';') {
			*out_ls = ls;
			*out_le = i + 1;
			return ESUCC;
		}

		// Found a comment. Ignore till the end of the line.
		if (buf[i] == '#') {
			for (i = i; i < size; ++i) {
				if (buf[i] == '\n')
					break;
			}
			continue;
		}

		// Else a : was found.
		assert(buf[i] == ':');
		for (j = ls; j < i; ++j) {
			if (isspace(buf[j]))
				return -EINVAL;
		}

		// Add the label to the current instruction.
		nl = ++in->num_labels;
		p = in->labels;
		in->labels = p = realloc(p, nl * sizeof(char *));
		p[nl - 1] = calloc(i - ls + 1, sizeof(char));
		memcpy(p[nl - 1], &buf[ls], i - ls);
		++i;
	}
}

int main(int argc, char **argv)
{
	int ls, le, i, err, num_instrs;
	char *buf;
	FILE *f;
	long size;
	struct instr *instrs, *in;

	if (argc != 2) {
		printf("Usage: %s input.s\n", argv[0]);
		return -EINVAL;
	}

	f = fopen(argv[1], "rb");
	if (f == NULL)
		return errno;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = malloc(size);
	if (buf == NULL)
		return ENOMEM;
	fread(buf, 1, size, f);
	fclose(f);

	num_instrs = 0;
	instrs = calloc(100, sizeof(*instrs));
	for (i = 0; i < size;) {
		in = &instrs[num_instrs];
		memset(in, 0, sizeof(*in));
		in->pc = num_instrs * 8;
		in->buf = buf;

		err = parse_labels(in, i, size, &le, &ls);
		if (err)
			break;
		i = le;

		if (ls == -1)
			continue;

		err = tokenize(buf, ls, le);
		if (err)
			break;
		print_tokens(buf);

		err = parse(in);
		if (err)
			break;

		++num_instrs;
		if (num_instrs % 100)
			continue;
		size = (num_instrs + 100) * sizeof(*instrs);
		instrs = realloc(instrs, size);
	}

	if (err)
		return err;

	for (i = 0; i < num_instrs; ++i) {
		in = &instrs[i];
		err = verify(in, instrs, num_instrs);
		if (err)
			break;
		err = encode(in);
		if (err)
			break;
		printf("0x%08x, 0x%08x,\n", in->lo, in->hi);
	}
	return err;
}
