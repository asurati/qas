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

#define MAX_TOKENS_PER_LINE		512

static
int is_label_reserved(const char *buf, int pos, int end)
{
	// TODO check against the reserved strings.
	(void)buf;
	(void)pos;
	(void)end;
	return 0;
}

// pos is at the start of the line.
// end points to either \n or the buffer end.
static
int parse_label(struct instr *instr, const char *buf, int pos, int end)
{
	int i, has_space, name_len;

	has_space = 0;
	for (i = pos; i < end; ++i) {
		// A line containing a label must have :
		if (buf[i] == ':')
			break;
		if (isspace(buf[i]))
			has_space = 1;
	}

	// Label not found.
	if (i == end)
		return ERR_NOT_FOUND;

	// This is a label. Verify its syntax.
	assert(buf[i] == ':');

	name_len = i - pos;

	// Shouldn't have a NULL name and shouldn't contain any spaces in its
	// name.
	if (name_len == 0 || has_space)
		return ERR_SYNTAX;

	// Add it to the list of labels for the current instruction.
	if (is_label_reserved(buf, pos, i))
		return ERR_SYNTAX;

	i = instr->num_labels++;
	instr->labels = realloc(instr->labels, (i + 1) * sizeof(char *));
	assert(instr->labels);
	instr->labels[i] = calloc(name_len + 1, sizeof(char));
	assert(instr->labels[i]);
	memcpy(instr->labels[i], &buf[pos], name_len);
	return ERR_SUCCESS;
}

// pos is at the start of the line.
// end points to either \n or the buffer end.
static
int parse_comment(const char *buf, int pos, int end)
{
	int i;

	// Check if the line begins with a # or a ; If so, it is a comment.
	// If the line contains nothing except spaces, treat it as a comment.

	// A comment.
	if (buf[pos] == '#' || buf[pos] == ';')
		return ERR_SUCCESS;

	// Ignore a line with all spaces as a comment.
	for (i = pos; i < end; ++i)
		if (!isspace(buf[i]))
			return ERR_NOT_FOUND;

	return ERR_SUCCESS;
}

// pos is the start of the line.
// end is \n or end of the buffer.
static
int tokenize_instr(struct instr *instr, const char *buf, int pos, int end)
{
	int i, token_start, k;

	instr->curr_token = -1;
	k = 0;
	token_start = -1;
	for (i = pos; i < end; ++i) {
		if (token_start < 0) {
			if (!isspace(buf[i]))
				token_start = i;
			continue;
		}

		// A token was in the midst of being scanned. If a space or a
		// delimiter was found, close the token.
		if (isspace(buf[i]) || buf[i] == '.' || buf[i] == ',' ||
		    buf[i] == ';') {
			assert(k < MAX_TOKENS_PER_LINE);

			instr->token_start[k] = token_start;
			instr->token_end[k] = i;
			++k;

			// If a delimiter was found, create a token for it too.
			if (!isspace(buf[i])) {
				assert(k < MAX_TOKENS_PER_LINE);
				instr->token_start[k] = i;
				instr->token_end[k] = i + 1;
				++k;

				// If the delim is a ;, end the tokenizer.
				if (buf[i] == ';') break;
			}

			// Not scanning a token at present.
			token_start = -1;
		}
	}

	// We must find a ;, else that is a syntax error.
	if (i == end)
		return ERR_SYNTAX;

	assert(k <= MAX_TOKENS_PER_LINE);
	instr->num_tokens = k;
	return ERR_SUCCESS;
}

int get_next_token(struct instr *instr, char *token, const char *buf)
{
	int tnum, ts, te;
	size_t tlen;

	if (instr->num_tokens <= 0)
		return ERR_SYNTAX;

	++instr->curr_token;

	if (instr->curr_token == instr->num_tokens)
		return ERR_SYNTAX;

	tnum = instr->curr_token;
	ts = instr->token_start[tnum];
	te = instr->token_end[tnum];
	tlen = te - ts;
	assert(tlen < 512);
	memcpy(token, &buf[ts], tlen);
	token[tlen] = 0;
	return ERR_SUCCESS;
}

int parse_op_code(const char *str, struct op *out_op)
{
	size_t i, j;
	const struct op_code_info *oi;
	static const struct op_code_info *ois[] = {
		g_op_codes_add,
		g_op_codes_mul,
		g_op_codes_li,
		g_op_codes_sem,
		g_op_codes_branch,
	};
	static const size_t oi_lens[] = {
		ARRAY_SIZE(g_op_codes_add),
		ARRAY_SIZE(g_op_codes_mul),
		ARRAY_SIZE(g_op_codes_li),
		ARRAY_SIZE(g_op_codes_sem),
		ARRAY_SIZE(g_op_codes_branch),
	};

	for (i = 0; i < 5; ++i) {
		oi = ois[i];
		for (j = 0; j < oi_lens[i]; ++j) {
			if (strcmp(oi[j].name, str))
				continue;
			out_op->type = (enum op_code_type)i;
			out_op->code  = oi[j].code;
			return ERR_SUCCESS;
		}
	}
	return ERR_SYNTAX;
}

int parse_cond_code(const char *str, enum cond_code *out_cc)
{
	size_t i, len;

	len = ARRAY_SIZE(g_cond_codes);
	for (i = 0; i < len; ++i) {
		if (strcmp(g_cond_codes[i].name, str))
			continue;
		*out_cc = g_cond_codes[i].code;
		return ERR_SUCCESS;
	}
	return ERR_SYNTAX;
}

static
int ishexdigit(char c)
{
	if (c >= '0' && c <= '9') return 1;
	if (c >= 'a' && c <= 'f') return 1;
	if (c >= 'A' && c <= 'F') return 1;
	return 0;
}

int parse_imm32(const char *str, int *out_imm)
{
	int i, len, hex, pos;
	unsigned int imm;

	len = strlen(str);
	if (len == 0)
		return ERR_SYNTAX;

	// First needs to be a digit.
	pos = 0;
	if (!isdigit(str[pos]))
		return ERR_SYNTAX;
	imm = str[pos] - '0';
	++pos;

	if (pos == len) {
		*out_imm = imm;
		return ERR_SUCCESS;
	}

	hex = 0;
	if (str[pos] == 'x') {
		if (imm)
			return ERR_SYNTAX;
		hex = 1;
		++pos;
		if (pos == len)
			return ERR_SYNTAX;
	}

	for (i = pos; i < len; ++i) {
		if (!hex && !isdigit(str[i]))
			return ERR_SYNTAX;
		if (hex && !ishexdigit(str[i]))
			return ERR_SYNTAX;

		if (!hex) {
			imm *= 10;
			imm += str[i] - '0';
		} else {
			imm <<= 4;
			if (str[i] >= '0' && str[i] <= '9')
				imm |= str[i] - '0';
			else
				imm |= 10 + str[i] - 'a';
		}
	}

	*out_imm = imm;
	return ERR_SUCCESS;
}

int parse_dst_reg(const char *str, struct reg *out_dst)
{
	size_t len, i, j;
	const struct reg_info *ri;
	static const struct reg_info *ris[] = {
		g_dst_regs_a,
		g_dst_regs_b,
		g_dst_regs_ab,
	};
	static const size_t ri_lens[] = {
		ARRAY_SIZE(g_dst_regs_a),
		ARRAY_SIZE(g_dst_regs_b),
		ARRAY_SIZE(g_dst_regs_ab),
	};

	for (i = 0; i < 3; ++i) {
		ri = ris[i];
		len = ri_lens[i];
		for (j = 0; j < len; ++j)
			if (!strcmp(ri[j].name, str))
				break;
		if (j == len)
			continue;

		out_dst->rf = (enum reg_file)i;
		out_dst->num = ri[j].num;
		return ERR_SUCCESS;
	}
	return ERR_SYNTAX;
}

static
void print_tokens(struct instr *instr, const char *buf)
{
	int i, j, ts, te;

	for (i = 0; i < instr->num_tokens; ++i) {
		ts = instr->token_start[i];
		te = instr->token_end[i];
		printf("\"");
		for (j = ts; j < te; ++j)
			printf("%c", buf[j]);
		printf("\"");
		if (i == instr->num_tokens - 1)
			printf("\n");
		else
			printf("   ");
	}
}

static
int parse_signal(const char *str, int *out_sig)
{
	size_t i, len;

	len = ARRAY_SIZE(g_signals);
	for (i = 0; i < len; ++i) {
		if (strcmp(g_signals[i].name, str))
			continue;
		*out_sig = g_signals[i].num;
		return ERR_SUCCESS;
	}
	return ERR_SYNTAX;
}

int resolve_dst_regfiles(struct instr *instr)
{
	int ws;
	enum reg_file rf[2];
	struct reg *d[2];

	d[0] = &instr->op[0].dst;
	d[1] = &instr->op[1].dst;

	rf[0] = d[0]->rf;
	rf[1] = d[1]->rf;

	// Add and Mul units both cannot write to the a (or b) simultaneously.
	if (rf[0] == rf[1] &&  rf[0] != RF_AB)
		return ERR_SYNTAX;

	// If add unit writes to regfile B, or if mul unit writes to regfile A,
	// enable write_swap.

	ws = 0;
	if (rf[0] == RF_B || rf[1] == RF_A)
		ws = 1;

	if (ws) {
		if (rf[0] == RF_AB) rf[0] = RF_B;
		if (rf[1] == RF_AB) rf[1] = RF_A;
	} else {
		if (rf[0] == RF_AB) rf[0] = RF_A;
		if (rf[1] == RF_AB) rf[1] = RF_B;
	}

	assert(rf[0] != rf[1]);

	// No need to change the reg num.
	d[0]->rf = rf[0];
	d[1]->rf = rf[1];
	instr->write_swap = ws;
	return ERR_SUCCESS;
}

static
int parse_instr(struct instr *instr, const char *buf)
{
	int err, signal;
	static char token[512];
	enum op_code_type type;

	instr->unpack = 0;
	instr->signal = 1;	// No Signal.
	instr->set_flag = instr->write_swap = 0;
	instr->op[0].cond_code = instr->op[1].cond_code = CC_DEFAULT;

	print_tokens(instr, buf);

	err = get_next_token(instr, token, buf);
	if (err) return err;

	err = parse_op_code(token, &instr->op[0]);
	if (err) return err;

	type = instr->op[0].type;
	switch (type) {
	case OP_TYPE_ADD:
		err = alu_parse_instr(instr, buf);
		break;
	case OP_TYPE_LI:
	case OP_TYPE_SEM:
		err = lisem_parse_instr(instr, buf);
		break;
	case OP_TYPE_BRANCH:
		err = branch_parse_instr(instr, buf);
		break;
	default:
		err = ERR_SYNTAX;
		break;
	}

	if (err) return err;

	// Now we expect sf,unpack,pack,sig; combinations.
	// Branch instructions do not support sf,unpack,pack,sig bits.
	if (type == OP_TYPE_BRANCH)
		return ERR_SUCCESS;

	for (;;) {
		err = get_next_token(instr, token, buf);
		if (err) return err;

		if (!strcmp(token, ";"))
			break;

		if (!strcmp(token, "sf")) {
			// Not more than one sf.
			if (instr->set_flag)
				return ERR_SYNTAX;
			instr->set_flag = 1;
			continue;
		}

		err = parse_signal(token, &signal);
		if (err) return err;

		// LI and SEM do not support unpack or sig.
		if (type == OP_TYPE_LI || type == OP_TYPE_SEM)
			return ERR_SYNTAX;

		// Can't have multiple signals.
		if (instr->signal != 1)
			return ERR_SYNTAX;
		instr->signal = signal;
	}
	return ERR_SUCCESS;
}

static
int gen_code(struct instr *instrs, int num_instrs, int curr_instr)
{
	int type, err;
	struct instr *instr;

	instr = &instrs[curr_instr];
	type = instr->op[0].type;

	switch (type) {
	case OP_TYPE_ADD:
		err = alu_resolve_instr(instr);
		break;
	case OP_TYPE_LI:
	case OP_TYPE_SEM:
		err = lisem_resolve_instr(instr);
		break;
	case OP_TYPE_BRANCH:
		err = branch_resolve_instr(instrs, num_instrs, instr);
		break;
	default:
		err = ERR_SYNTAX;
		break;
	}

	if (err) return err;

	// Resolve may change the type. Re-read.
	type = instr->op[0].type;
	switch (type) {
	case OP_TYPE_ADD:
		alu_gen_code(instr);
		break;
	case OP_TYPE_LI:
	case OP_TYPE_SEM:
		lisem_gen_code(instr);
		break;
	case OP_TYPE_BRANCH:
		branch_gen_code(instr);
		break;
	default:
		return ERR_SYNTAX;
	}

	printf("enc%d: 0x%x, 0x%x,\n", curr_instr, instr->encoding[0],
	       instr->encoding[1]);
	return ERR_SUCCESS;
}

int main(int argc, char **argv)
{
	struct instr *instrs, *instr;
	int pos, err, i, num_instrs;
	FILE *f;
	int size;
	char *buf;
	static int token_start[MAX_TOKENS_PER_LINE];
	static int token_end[MAX_TOKENS_PER_LINE];

	if (argc != 2) {
		printf("Usage: %s input.s\n", argv[0]);
		return ERR_PARAM;
	}

	f = fopen(argv[1], "rb");
	if (f == NULL)
		return ERR_PARAM;
	fseek(f, 0, SEEK_END);	// Non portable.
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	buf = malloc(size);
	assert(buf);
	fread(buf, 1, size, f);
	fclose(f);

	// Allocate 100 for now.
	instrs = malloc(100 * sizeof(*instrs));

	num_instrs = 0;
	instrs[num_instrs].num_labels = 0;
	for (pos = 0; pos < size; pos = i + 1) {

		// Find the line.
		for (i = pos; i < size; ++i)
			if (buf[i] == '\n')
				break;

		assert(i == size || buf[i] == '\n');

		err = parse_comment(buf, pos, i);
		if (!err) continue;
		else if (err == ERR_SYNTAX) break;

		instr = &instrs[num_instrs];

		err = parse_label(instr, buf, pos, i);
		if (!err) continue;
		else if (err == ERR_SYNTAX) break;

		// PC contains the address and not a instruction number, as
		// the PC+4 notation in the spec would have one believe.

		instr->pc = num_instrs << 3;	// 64-bit ILen.

		// Static arrays for store token information.
		instr->token_start = token_start;
		instr->token_end = token_end;

		err = tokenize_instr(instr, buf, pos, i);
		if (err) break;

		err = parse_instr(instr, buf);
		if (err) break;

		++num_instrs;
		if (num_instrs && (num_instrs % 100) == 0)
			instrs = realloc(instrs, (num_instrs + 100) *
					 sizeof(*instrs));
		instrs[num_instrs].num_labels = 0;
	}

	for (i = 0; !err && i < num_instrs; ++i)
		err = gen_code(instrs, num_instrs, i);

	printf("main ret %d\n", err);
	return err;
}
