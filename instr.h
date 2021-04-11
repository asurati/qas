// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2021 Amol Surati

#ifndef INSTR_H
#define INSTR_H

#include <stdint.h>

#include "bits.h"

#define ARRAY_SIZE(a)			(sizeof(a) / sizeof((a)[0]))

enum reg_file {
	RF_A,
	RF_B,
	RF_AB,
	RF_R,	// accumulators
	RF_SIMM
};

struct reg_info {
	const char			*name;
	int				num;
};

#define DEFINE_REG(file, num)	{file#num, num}
#define DEFINE_REG_A(num)	DEFINE_REG("a", num)
#define DEFINE_REG_B(num)	DEFINE_REG("b", num)
#define DEFINE_REG_R(num)	DEFINE_REG("r", num)

static
const struct reg_info g_src_regs_r[] = {
	DEFINE_REG_R(0), DEFINE_REG_R(1), DEFINE_REG_R(2), DEFINE_REG_R(3),
	DEFINE_REG_R(4), DEFINE_REG_R(5),
};

static
const struct reg_info g_src_regs_a[] = {
	DEFINE_REG_A(0), DEFINE_REG_A(1), DEFINE_REG_A(2), DEFINE_REG_A(3),
	DEFINE_REG_A(4), DEFINE_REG_A(5), DEFINE_REG_A(6), DEFINE_REG_A(7),
	DEFINE_REG_A(8), DEFINE_REG_A(9), DEFINE_REG_A(10),DEFINE_REG_A(11),
	DEFINE_REG_A(12),DEFINE_REG_A(13),DEFINE_REG_A(14),DEFINE_REG_A(15),
	DEFINE_REG_A(16),DEFINE_REG_A(17),DEFINE_REG_A(18),DEFINE_REG_A(19),
	DEFINE_REG_A(20),DEFINE_REG_A(21),DEFINE_REG_A(22),DEFINE_REG_A(23),
	DEFINE_REG_A(24),DEFINE_REG_A(25),DEFINE_REG_A(26),DEFINE_REG_A(27),
	DEFINE_REG_A(28),DEFINE_REG_A(29),DEFINE_REG_A(30),DEFINE_REG_A(31),

	{"ele",		38},
	{"x",		41},
	{"msf",		42},
	{"ldbusy",	49},
	{"ldwait",	50},
};

static
const struct reg_info g_src_regs_b[] = {
	DEFINE_REG_B(0), DEFINE_REG_B(1), DEFINE_REG_B(2), DEFINE_REG_B(3),
	DEFINE_REG_B(4), DEFINE_REG_B(5), DEFINE_REG_B(6), DEFINE_REG_B(7),
	DEFINE_REG_B(8), DEFINE_REG_B(9), DEFINE_REG_B(10),DEFINE_REG_B(11),
	DEFINE_REG_B(12),DEFINE_REG_B(13),DEFINE_REG_B(14),DEFINE_REG_B(15),
	DEFINE_REG_B(16),DEFINE_REG_B(17),DEFINE_REG_B(18),DEFINE_REG_B(19),
	DEFINE_REG_B(20),DEFINE_REG_B(21),DEFINE_REG_B(22),DEFINE_REG_B(23),
	DEFINE_REG_B(24),DEFINE_REG_B(25),DEFINE_REG_B(26),DEFINE_REG_B(27),
	DEFINE_REG_B(28),DEFINE_REG_B(29),DEFINE_REG_B(30),DEFINE_REG_B(31),

	{"qpu",		38},
	{"y",		41},
	{"revf",	42},
	{"stbusy",	49},
	{"stwait",	50},
};

static
const struct reg_info g_src_regs_ab[] = {
	{"unif",	32},
	{"vary",	35},
	{"-",		39},
	{"vpm",		48},
	{"mtxacq",	51},
};

static
const struct reg_info g_src_regs_simm[] = {
	{"0",		0},
	{"1",		1},
	{"2",		2},
	{"3",		3},
	{"4",		4},
	{"5",		5},
	{"6",		6},
	{"7",		7},
	{"8",		8},
	{"9",		9},
	{"10",		10},
	{"11",		11},
	{"12",		12},
	{"13",		13},
	{"14",		14},
	{"15",		15},

	{"-16",		16},
	{"-15",		17},
	{"-14",		18},
	{"-13",		19},
	{"-12",		20},
	{"-11",		21},
	{"-10",		22},
	{"-9",		23},
	{"-8",		24},
	{"-7",		25},
	{"-6",		26},
	{"-5",		27},
	{"-4",		28},
	{"-3",		29},
	{"-2",		30},
	{"-1",		31},

	// Can't use 1.0 etc, since . is a delimiter.
	{"1f",		32},
	{"2f",		33},
	{"4f",		34},
	{"8f",		35},
	{"16f",		36},
	{"32f",		37},
	{"64f",		38},
	{"128f",	39},

	{"i256f",	40},
	{"i128f",	41},
	{"i64f",	42},
	{"i32f",	43},
	{"i16f",	44},
	{"i8f",		45},
	{"i4f",		46},
	{"i2f",		47},

	{"rotr5",	48},
	{"rot1",	49},
	{"rot2",	50},
	{"rot3",	51},
	{"rot4",	52},
	{"rot5",	53},
	{"rot6",	54},
	{"rot7",	55},
	{"rot8",	56},
	{"rot9",	57},
	{"rot10",	58},
	{"rot11",	59},
	{"rot12",	60},
	{"rot13",	61},
	{"rot14",	62},
	{"rot15",	63},
};

static
const struct reg_info g_dst_regs_a[] = {
	DEFINE_REG_A(0), DEFINE_REG_A(1), DEFINE_REG_A(2), DEFINE_REG_A(3),
	DEFINE_REG_A(4), DEFINE_REG_A(5), DEFINE_REG_A(6), DEFINE_REG_A(7),
	DEFINE_REG_A(8), DEFINE_REG_A(9), DEFINE_REG_A(10),DEFINE_REG_A(11),
	DEFINE_REG_A(12),DEFINE_REG_A(13),DEFINE_REG_A(14),DEFINE_REG_A(15),
	DEFINE_REG_A(16),DEFINE_REG_A(17),DEFINE_REG_A(18),DEFINE_REG_A(19),
	DEFINE_REG_A(20),DEFINE_REG_A(21),DEFINE_REG_A(22),DEFINE_REG_A(23),
	DEFINE_REG_A(24),DEFINE_REG_A(25),DEFINE_REG_A(26),DEFINE_REG_A(27),
	DEFINE_REG_A(28),DEFINE_REG_A(29),DEFINE_REG_A(30),DEFINE_REG_A(31),

	{"qx",		41},
	{"msf",		42},
	{"vpmrds",	49},
	{"ldaddr",	50},
};

static
const struct reg_info g_dst_regs_b[] = {
	DEFINE_REG_B(0), DEFINE_REG_B(1), DEFINE_REG_B(2), DEFINE_REG_B(3),
	DEFINE_REG_B(4), DEFINE_REG_B(5), DEFINE_REG_B(6), DEFINE_REG_B(7),
	DEFINE_REG_B(8), DEFINE_REG_B(9), DEFINE_REG_B(10),DEFINE_REG_B(11),
	DEFINE_REG_B(12),DEFINE_REG_B(13),DEFINE_REG_B(14),DEFINE_REG_B(15),
	DEFINE_REG_B(16),DEFINE_REG_B(17),DEFINE_REG_B(18),DEFINE_REG_B(19),
	DEFINE_REG_B(20),DEFINE_REG_B(21),DEFINE_REG_B(22),DEFINE_REG_B(23),
	DEFINE_REG_B(24),DEFINE_REG_B(25),DEFINE_REG_B(26),DEFINE_REG_B(27),
	DEFINE_REG_B(28),DEFINE_REG_B(29),DEFINE_REG_B(30),DEFINE_REG_B(31),

	{"qy",		41},
	{"revf",	42},
	{"vpmwrs",	49},
	{"staddr",	50},
};

static
const struct reg_info g_dst_regs_ab[] = {
	{"r0",		32},
	{"r1",		33},
	{"r2",		34},
	{"r3",		35},
	{"tmunswp",	36},
	{"r5",		37},
	{"hostint",	38},
	{"-",		39},
	{"unif",	40},
	{"tlbss",	43},
	{"tlbz",	44},
	{"tlbcms",	45},
	{"tlbcall",	46},
	{"tlbalpha",	47},
	{"vpm",		48},
	{"mtxrel",	51},
	{"rcp",		52},
	{"rcpsqrt",	53},
	{"exp",		54},
	{"log",		55},
	{"tmu0s",	56},
	{"tmu0t",	57},
	{"tmu0r",	58},
	{"tmu0b",	59},
	{"tmu1s",	60},
	{"tmu1t",	61},
	{"tmu1r",	62},
	{"tmu1b",	63},
};

struct reg {
	enum reg_file			rf;
	int				num;
};

enum src_type {
	ST_IMM32,
	ST_LABEL,
	ST_REG,
};

struct src {
	enum src_type			type;
	union {
		uint32_t		imm32;
		const char		*label;
		struct reg		reg;
	} u;
};






////////////////////////////////////////////////////////////////////////
struct signal_info {
	const char			*name;
	int				num;
};

static
const struct signal_info g_signals[] = {
	{"bkpt",	0},
	{"swt",		2},
	{"endp",	3},
	{"wtsb",	4},
	{"ulsb",	5},
	{"swtl",	6},
	{"ldcvr",	7},
	{"ldclr",	8},
	{"ldclrep",	9},
	{"ldtm0",	10},
	{"ldtm1",	11},
	{"ldalpha",	12},
};








////////////////////////////////////////////////////////////////////////
enum cond_code {
	CC_NEVER,
	CC_ALWAYS,
	CC_Z,
	CC_NZ,
	CC_N,
	CC_NN,
	CC_C,
	CC_NC,
	CC_DEFAULT,
	CC_NUM = CC_DEFAULT,
};

struct cond_code_info {
	const char			*name;
	enum cond_code			code;
};

static
const struct cond_code_info g_cond_codes[] = {
	{"x",		CC_NEVER},
	{"a",		CC_ALWAYS},
	{"z",		CC_Z},
	{"nz",		CC_NZ},
	{"n",		CC_N},
	{"nn",		CC_NN},
	{"c",		CC_C},
	{"nc",		CC_NC},
};





////////////////////////////////////////////////////////////////////////
// TODO pack/unpack.

////////////////////////////////////////////////////////////////////////
enum op_code_type {
	OP_TYPE_ADD,
	OP_TYPE_MUL,
	OP_TYPE_LI,
	OP_TYPE_SEM,
	OP_TYPE_BRANCH,
};

enum op_code_li {
	OP_LI,
	OP_LIS,
	OP_LIU,
};

enum op_code_sem {
	OP_SEMUP,
	OP_SEMDN,
};

enum op_code_branch {
	OP_B,
	OP_BZ,
	OP_BNZ,
	OP_BN,
	OP_BNN,
	OP_BC,
	OP_BNC,
};

struct op_code_info {
	const char			*name;
	int				code;
};

static
const struct op_code_info g_op_codes_add[] = {
	{"-",		0},
	{"fadd",	1},
	{"fsub",	2},
	{"fmin",	3},
	{"fmax",	4},
	{"fminabs",	5},
	{"fmaxabs",	6},
	{"ftoi",	7},
	{"itof",	8},

	{"add",		12},
	{"sub",		13},
	{"shr",		14},
	{"asr",		15},
	{"ror",		16},
	{"shl",		17},
	{"min",		18},
	{"max",		19},
	{"and",		20},
	{"or",		21},
	{"xor",		22},
	{"not",		23},
	{"clz",		24},

	{"v8adds",	30},
	{"v8subs",	31},
};

static
const struct op_code_info g_op_codes_mul[] = {
	{"-",		0},
	{"fmul",	1},
	{"mul24",	2},
	{"v8muld",	3},
	{"v8min",	4},
	{"v8max",	5},
	{"v8adds",	6},
	{"v8subs",	7},
};

static
const struct op_code_info g_op_codes_li[] = {
	{"li",		OP_LI},
	{"lis",		OP_LIS},
	{"liu",		OP_LIU},
};

static
const struct op_code_info g_op_codes_sem[] = {
	{"semup",	OP_SEMUP},
	{"semdn",	OP_SEMDN},
};

static
const struct op_code_info g_op_codes_branch[] = {
	{"b",		OP_B},
	{"bz",		OP_BZ},
	{"bnz",		OP_BNZ},
	{"bn",		OP_BN},
	{"bnn",		OP_BNN},
	{"bc",		OP_BC},
	{"bnc",		OP_BNC},
};

struct op {
	enum op_code_type		type;
	int				code;
	enum cond_code			cond_code;
	struct reg			dst;
	struct src			src[2];
};






////////////////////////////////////////////////////////////////////////
struct instr {
	char				**labels;
	int				num_labels;
	int				pc;

	struct op			op[2];
	int				signal;
	int				set_flag;
	int				write_swap;
	int				branch_rel;
	int				branch_reg;
	unsigned int			encoding[2];

	int				*token_start;
	int				*token_end;
	int				num_tokens;
	int				curr_token;
	int				unpack;
};

#define MUL_B_POS			0
#define MUL_A_POS			3
#define ADD_B_POS			6
#define ADD_A_POS			9
#define RADDR_B_POS			12
#define RADDR_A_POS			18
#define ADD_OP_POS			24
#define MUL_OP_POS			29

#define MUL_DST_POS			0
#define ADD_DST_POS			6
#define WS_POS				12
#define SF_POS				13
#define MUL_COND_POS			14
#define ADD_COND_POS			17
#define PACK_POS			20
#define PM_POS				24
#define UNPACK_POS			25
#define SIG_POS				28

#define MUL_B_BITS			3
#define MUL_A_BITS			3
#define ADD_B_BITS			3
#define ADD_A_BITS			3
#define RADDR_B_BITS			6
#define RADDR_A_BITS			6
#define ADD_OP_BITS			5
#define MUL_OP_BITS			3

#define MUL_DST_BITS			6
#define ADD_DST_BITS			6
#define WS_BITS				1
#define SF_BITS				1
#define MUL_COND_BITS			3
#define ADD_COND_BITS			3
#define PACK_BITS			4
#define PM_BITS				1
#define UNPACK_BITS			3
#define SIG_BITS			4

#define IMM_POS				0
#define IMM_BITS			32

#define BR_RADDR_A_POS			13
#define BR_REG_POS			18
#define BR_REL_POS			19
#define BR_RADDR_A_BITS			5
#define BR_REG_BITS			1
#define BR_REL_BITS			1

int	get_next_token(struct instr *instr, char *str, const char *buf);
int	parse_op_code(const char *str, struct op *out_op);
int	parse_cond_code(const char *str, enum cond_code *out_cc);
int	parse_imm32(const char *str, int *out_imm);
int	parse_dst_reg(const char *str, struct reg *out_dst);
int	resolve_dst_regfiles(struct instr *instr);

int	alu_parse_instr(struct instr *instr, const char *buf);
int	alu_resolve_instr(struct instr *instr);
void	alu_gen_code(struct instr *instr);

int	lisem_parse_instr(struct instr *instr, const char *buf);
int	lisem_resolve_instr(struct instr *instr);
void	lisem_gen_code(struct instr *instr);

int	branch_parse_instr(struct instr *instr, const char *buf);
int	branch_resolve_instr(struct instr *instrs, int num_instrs,
			     struct instr *instr);
void	branch_gen_code(struct instr *instr);
#endif
