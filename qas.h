// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2021 Amol Surati

#ifndef QAS_H
#define QAS_H

#include <stdint.h>
#include <errno.h>

#include "bits.h"

#define NUM_ARR(a)			((int)(sizeof(a)/sizeof(a[0])))

#define ESUCC				0
#define MAX_TOKENS			256

enum op_code {
	OP_INVALID,
	OP_NOP,

	OP_ADD_FADD,
	OP_ADD_FSUB,
	OP_ADD_FMIN,
	OP_ADD_FMAX,
	OP_ADD_FMINABS,
	OP_ADD_FMAXABS,
	OP_ADD_FTOI,
	OP_ADD_ITOF,
	OP_ADD_ADD,
	OP_ADD_SUB,
	OP_ADD_SHR,
	OP_ADD_ASR,
	OP_ADD_ROR,
	OP_ADD_SHL,
	OP_ADD_MIN,
	OP_ADD_MAX,
	OP_ADD_AND,
	OP_ADD_OR,
	OP_ADD_XOR,
	OP_ADD_NOT,
	OP_ADD_CLZ,
	OP_ADD_V8ADDS,
	OP_ADD_V8SUBS,
	OP_MUL_FMUL,
	OP_MUL_MUL24,
	OP_MUL_V8MULD,
	OP_MUL_V8MIN,
	OP_MUL_V8MAX,

	OP_ADD_FADDI,
	OP_ADD_FSUBI,
	OP_ADD_FMINI,
	OP_ADD_FMAXI,
	OP_ADD_FMINABSI,
	OP_ADD_FMAXABSI,
	OP_ADD_FTOII,
	OP_ADD_ITOFI,
	OP_ADD_ADDI,
	OP_ADD_SUBI,
	OP_ADD_SHRI,
	OP_ADD_ASRI,
	OP_ADD_RORI,
	OP_ADD_SHLI,
	OP_ADD_MINI,
	OP_ADD_MAXI,
	OP_ADD_ANDI,
	OP_ADD_ORI,
	OP_ADD_XORI,
	OP_ADD_NOTI,
	OP_ADD_CLZI,
	OP_ADD_V8ADDSI,
	OP_ADD_V8SUBSI,
	OP_MUL_FMULI,
	OP_MUL_MUL24I,
	OP_MUL_V8MULDI,
	OP_MUL_V8MINI,
	OP_MUL_V8MAXI,

	OP_MUL_V8ADDS_ROTR5,
	OP_MUL_V8ADDS_ROT1,
	OP_MUL_V8ADDS_ROT2,
	OP_MUL_V8ADDS_ROT3,
	OP_MUL_V8ADDS_ROT4,
	OP_MUL_V8ADDS_ROT5,
	OP_MUL_V8ADDS_ROT6,
	OP_MUL_V8ADDS_ROT7,
	OP_MUL_V8ADDS_ROT8,
	OP_MUL_V8ADDS_ROT9,
	OP_MUL_V8ADDS_ROT10,
	OP_MUL_V8ADDS_ROT11,
	OP_MUL_V8ADDS_ROT12,
	OP_MUL_V8ADDS_ROT13,
	OP_MUL_V8ADDS_ROT14,
	OP_MUL_V8ADDS_ROT15,

	OP_BR_B,
	OP_BR_BL,

	OP_IMM_LI,
	OP_IMM_LIS,
	OP_IMM_LIU,

	OP_SEM_SEMUP,
	OP_SEM_SEMDN,

	OP_SIG_BREAK,
	OP_SIG_NONE,
	OP_SIG_THRD_SWITCH,
	OP_SIG_PROG_END,
	OP_SIG_WAIT_SB,
	OP_SIG_UNLOCK_SB,
	OP_SIG_LAST_THRD_SWITCH,
	OP_SIG_COVERAGE,
	OP_SIG_COLOUR,
	OP_SIG_COLOUR_PROG_END,
	OP_SIG_LD_TMU0,
	OP_SIG_LD_TMU1,
	OP_SIG_LD_ALPHA,
	OP_FLAGS_SF,
	OP_FLAGS_PM,

	// Add pack/unpack flags here.

	// These signals cannot be specified explicitly.
	// They are used to set in->sig.
	OP_SIG_SIMM,
	OP_SIG_LI,
	OP_SIG_BR,
};

enum reg_file {
	RF_A,
	RF_B,
	RF_AB,
	RF_SIMM,
	RF_ACC,
	RF_IMM,
};

enum cc {
	CC_NEVER,
	CC_ALWAYS,

	CC_Z,
	CC_NZ,
	CC_N,
	CC_NN,
	CC_C,
	CC_NC,

	CC_ALL_Z,
	CC_ALL_NZ,
	CC_ALL_N,
	CC_ALL_NN,
	CC_ALL_C,
	CC_ALL_NC,
};

struct reg_info {
	const char			*name;
	enum reg_file			rf;
	int				num;
};

struct op_info {
	const char			*name;
	enum op_code			code;
};

struct cc_info {
	const char			*name;
	enum cc				code;
};

static
const struct reg_info g_src_reg_info[] = {
	{"a0",			RF_A,		0},
	{"a1",			RF_A,		1},
	{"a2",			RF_A,		2},
	{"a3",			RF_A,		3},
	{"a4",			RF_A,		4},
	{"a5",			RF_A,		5},
	{"a6",			RF_A,		6},
	{"a7",			RF_A,		7},
	{"a8",			RF_A,		8},
	{"a9",			RF_A,		9},
	{"a10",			RF_A,		10},
	{"a11",			RF_A,		11},
	{"a12",			RF_A,		12},
	{"a13",			RF_A,		13},
	{"a14",			RF_A,		14},
	{"a15",			RF_A,		15},
	{"a16",			RF_A,		16},
	{"a17",			RF_A,		17},
	{"a18",			RF_A,		18},
	{"a19",			RF_A,		19},
	{"a20",			RF_A,		20},
	{"a21",			RF_A,		21},
	{"a22",			RF_A,		22},
	{"a23",			RF_A,		23},
	{"a24",			RF_A,		24},
	{"a25",			RF_A,		25},
	{"a26",			RF_A,		26},
	{"a27",			RF_A,		27},
	{"a28",			RF_A,		28},
	{"a29",			RF_A,		29},
	{"a30",			RF_A,		30},
	{"a31",			RF_A,		31},

	{"b0",			RF_B,		0},
	{"b1",			RF_B,		1},
	{"b2",			RF_B,		2},
	{"b3",			RF_B,		3},
	{"b4",			RF_B,		4},
	{"b5",			RF_B,		5},
	{"b6",			RF_B,		6},
	{"b7",			RF_B,		7},
	{"b8",			RF_B,		8},
	{"b9",			RF_B,		9},
	{"b10",			RF_B,		10},
	{"b11",			RF_B,		11},
	{"b12",			RF_B,		12},
	{"b13",			RF_B,		13},
	{"b14",			RF_B,		14},
	{"b15",			RF_B,		15},
	{"b16",			RF_B,		16},
	{"b17",			RF_B,		17},
	{"b18",			RF_B,		18},
	{"b19",			RF_B,		19},
	{"b20",			RF_B,		20},
	{"b21",			RF_B,		21},
	{"b22",			RF_B,		22},
	{"b23",			RF_B,		23},
	{"b24",			RF_B,		24},
	{"b25",			RF_B,		25},
	{"b26",			RF_B,		26},
	{"b27",			RF_B,		27},
	{"b28",			RF_B,		28},
	{"b29",			RF_B,		29},
	{"b30",			RF_B,		30},
	{"b31",			RF_B,		31},

	{"r0",			RF_ACC,		0},
	{"r1",			RF_ACC,		1},
	{"r2",			RF_ACC,		2},
	{"r3",			RF_ACC,		3},
	{"r4",			RF_ACC,		4},
	{"r5",			RF_ACC,		5},

	{"uni_rd",		RF_AB,		32},
	{"vary_rd",		RF_AB,		35},
	{"ele_num",		RF_A,		38},
	{"qpu_num",		RF_B,		38},
	{"-",			RF_AB,		39},
	{"x_px_coord",		RF_A,		41},
	{"y_px_coord",		RF_B,		41},
	{"ms_flags",		RF_A,		42},
	{"rev_flag",		RF_B,		42},

	{"vpm_rd",		RF_AB,		48},
	{"vpr",			RF_AB,		48},

	{"vpm_ld_busy",		RF_A,		49},
	{"vpm_st_busy",		RF_B,		49},

	{"vpm_ld_wait",		RF_A,		50},
	{"vdr_wait",		RF_A,		50},

	{"vpm_st_wait",		RF_B,		50},
	{"vdw_wait",		RF_B,		50},

	{"mtx_acq",		RF_AB,		51},

	// Small immediates
	{"0",			RF_SIMM,	0},
	{"1",			RF_SIMM,	1},
	{"2",			RF_SIMM,	2},
	{"3",			RF_SIMM,	3},
	{"4",			RF_SIMM,	4},
	{"5",			RF_SIMM,	5},
	{"6",			RF_SIMM,	6},
	{"7",			RF_SIMM,	7},
	{"8",			RF_SIMM,	8},
	{"9",			RF_SIMM,	9},
	{"10",			RF_SIMM,	10},
	{"11",			RF_SIMM,	11},
	{"12",			RF_SIMM,	12},
	{"13",			RF_SIMM,	13},
	{"14",			RF_SIMM,	14},
	{"15",			RF_SIMM,	15},
	{"-16",			RF_SIMM,	16},
	{"-15",			RF_SIMM,	17},
	{"-14",			RF_SIMM,	18},
	{"-13",			RF_SIMM,	19},
	{"-12",			RF_SIMM,	20},
	{"-11",			RF_SIMM,	21},
	{"-10",			RF_SIMM,	22},
	{"-9",			RF_SIMM,	23},
	{"-8",			RF_SIMM,	24},
	{"-7",			RF_SIMM,	25},
	{"-6",			RF_SIMM,	26},
	{"-5",			RF_SIMM,	27},
	{"-4",			RF_SIMM,	28},
	{"-3",			RF_SIMM,	29},
	{"-2",			RF_SIMM,	30},
	{"-1",			RF_SIMM,	31},
	// Can't use 1.0, etc. since . is a delimiter.
	{"1f",			RF_SIMM,	32},
	{"2f",			RF_SIMM,	33},
	{"4f",			RF_SIMM,	34},
	{"8f",			RF_SIMM,	35},
	{"16f",			RF_SIMM,	36},
	{"32f",			RF_SIMM,	37},
	{"64f",			RF_SIMM,	38},
	{"128f",		RF_SIMM,	39},
	{"i256f",		RF_SIMM,	40},
	{"i128f",		RF_SIMM,	41},
	{"i64f",		RF_SIMM,	42},
	{"i32f",		RF_SIMM,	43},
	{"i16f",		RF_SIMM,	44},
	{"i8f",			RF_SIMM,	45},
	{"i4f",			RF_SIMM,	46},
	{"i2f",			RF_SIMM,	47},
};

static
const struct reg_info g_dst_reg_info[] = {
	{"a0",			RF_A,		0},
	{"a1",			RF_A,		1},
	{"a2",			RF_A,		2},
	{"a3",			RF_A,		3},
	{"a4",			RF_A,		4},
	{"a5",			RF_A,		5},
	{"a6",			RF_A,		6},
	{"a7",			RF_A,		7},
	{"a8",			RF_A,		8},
	{"a9",			RF_A,		9},
	{"a10",			RF_A,		10},
	{"a11",			RF_A,		11},
	{"a12",			RF_A,		12},
	{"a13",			RF_A,		13},
	{"a14",			RF_A,		14},
	{"a15",			RF_A,		15},
	{"a16",			RF_A,		16},
	{"a17",			RF_A,		17},
	{"a18",			RF_A,		18},
	{"a19",			RF_A,		19},
	{"a20",			RF_A,		20},
	{"a21",			RF_A,		21},
	{"a22",			RF_A,		22},
	{"a23",			RF_A,		23},
	{"a24",			RF_A,		24},
	{"a25",			RF_A,		25},
	{"a26",			RF_A,		26},
	{"a27",			RF_A,		27},
	{"a28",			RF_A,		28},
	{"a29",			RF_A,		29},
	{"a30",			RF_A,		30},
	{"a31",			RF_A,		31},

	{"b0",			RF_B,		0},
	{"b1",			RF_B,		1},
	{"b2",			RF_B,		2},
	{"b3",			RF_B,		3},
	{"b4",			RF_B,		4},
	{"b5",			RF_B,		5},
	{"b6",			RF_B,		6},
	{"b7",			RF_B,		7},
	{"b8",			RF_B,		8},
	{"b9",			RF_B,		9},
	{"b10",			RF_B,		10},
	{"b11",			RF_B,		11},
	{"b12",			RF_B,		12},
	{"b13",			RF_B,		13},
	{"b14",			RF_B,		14},
	{"b15",			RF_B,		15},
	{"b16",			RF_B,		16},
	{"b17",			RF_B,		17},
	{"b18",			RF_B,		18},
	{"b19",			RF_B,		19},
	{"b20",			RF_B,		20},
	{"b21",			RF_B,		21},
	{"b22",			RF_B,		22},
	{"b23",			RF_B,		23},
	{"b24",			RF_B,		24},
	{"b25",			RF_B,		25},
	{"b26",			RF_B,		26},
	{"b27",			RF_B,		27},
	{"b28",			RF_B,		28},
	{"b29",			RF_B,		29},
	{"b30",			RF_B,		30},
	{"b31",			RF_B,		31},

	{"r0",			RF_AB,		32},
	{"r1",			RF_AB,		33},
	{"r2",			RF_AB,		34},
	{"r3",			RF_AB,		35},
	{"tmu_noswap",		RF_AB,		36},
	{"r5",			RF_AB,		37},
	{"host_int",		RF_AB,		38},
	{"-",			RF_AB,		39},
	{"uni_addr",		RF_AB,		40},
	{"quad_x",		RF_A,		41},
	{"quad_y",		RF_B,		41},
	{"ms_flags",		RF_A,		42},
	{"rev_flag",		RF_B,		42},
	{"tlb_stencil",		RF_AB,		43},
	{"tlb_z",		RF_AB,		44},
	{"tlb_clr_ms",		RF_AB,		45},
	{"tlb_clr_all",		RF_AB,		46},
	{"tlb_amask",		RF_AB,		47},

	{"vpm_wr",		RF_AB,		48},
	{"vpw",			RF_AB,		48},

	{"vpm_rd_setup",	RF_A,		49},
	{"vdr_setup",		RF_A,		49},
	{"vpr_setup",		RF_A,		49},

	{"vpm_wr_setup",	RF_B,		49},
	{"vdw_setup",		RF_B,		49},
	{"vpw_setup",		RF_B,		49},

	{"vpm_ld_addr",		RF_A,		50},
	{"vdr_addr",		RF_A,		50},

	{"vpm_st_addr",		RF_B,		50},
	{"vdw_addr",		RF_B,		50},

	{"mtx_rel",		RF_AB,		51},
	{"sfu_recip",		RF_AB,		52},
	{"sfu_rsqrt",		RF_AB,		53},
	{"sfu_exp",		RF_AB,		54},
	{"sfu_log",		RF_AB,		55},
	{"tmu0_s",		RF_AB,		56},
	{"tmu0_t",		RF_AB,		57},
	{"tmu0_r",		RF_AB,		58},
	{"tmu0_b",		RF_AB,		59},
	{"tmu1_s",		RF_AB,		60},
	{"tmu1_t",		RF_AB,		61},
	{"tmu1_r",		RF_AB,		62},
	{"tmu1_b",		RF_AB,		63},
};

static
const struct cc_info g_cc_info[] = {
	{"x",		CC_NEVER},
	{"a",		CC_ALWAYS},
	{"z",		CC_Z},
	{"nz",		CC_NZ},
	{"n",		CC_N},
	{"nn",		CC_NN},
	{"c",		CC_C},
	{"nc",		CC_NC},

	{"zl",		CC_ALL_Z},
	{"nzl",		CC_ALL_NZ},
	{"nl",		CC_ALL_N},
	{"nnl",		CC_ALL_NN},
	{"cl",		CC_ALL_C},
	{"ncl",		CC_ALL_NC},
};

static
const struct op_info g_op_info[] = {
	{"fadd",	OP_ADD_FADD},
	{"fsub",	OP_ADD_FSUB},
	{"fmin",	OP_ADD_FMIN},
	{"fmax",	OP_ADD_FMAX},
	{"fminabs",	OP_ADD_FMINABS},
	{"fmaxabs",	OP_ADD_FMAXABS},
	{"ftoi",	OP_ADD_FTOI},
	{"itof",	OP_ADD_ITOF},
	{"add",		OP_ADD_ADD},
	{"sub",		OP_ADD_SUB},
	{"shr",		OP_ADD_SHR},
	{"asr",		OP_ADD_ASR},
	{"ror",		OP_ADD_ROR},
	{"shl",		OP_ADD_SHL},
	{"min",		OP_ADD_MIN},
	{"max",		OP_ADD_MAX},
	{"and",		OP_ADD_AND},
	{"or",		OP_ADD_OR},
	{"xor",		OP_ADD_XOR},
	{"not",		OP_ADD_NOT},
	{"clz",		OP_ADD_CLZ},
	{"v8adds",	OP_ADD_V8ADDS},
	{"v8subs",	OP_ADD_V8SUBS},
	{"fmul",	OP_MUL_FMUL},
	{"mul24",	OP_MUL_MUL24},
	{"v8muld",	OP_MUL_V8MULD},
	{"v8min",	OP_MUL_V8MIN},
	{"v8max",	OP_MUL_V8MAX},

	{"faddi",	OP_ADD_FADDI},
	{"fsubi",	OP_ADD_FSUBI},
	{"fmini",	OP_ADD_FMINI},
	{"fmaxi",	OP_ADD_FMAXI},
	{"fminabsi",	OP_ADD_FMINABSI},
	{"fmaxabsi",	OP_ADD_FMAXABSI},
	{"ftoii",	OP_ADD_FTOII},
	{"itofi",	OP_ADD_ITOFI},
	{"addi",	OP_ADD_ADDI},
	{"subi",	OP_ADD_SUBI},
	{"shri",	OP_ADD_SHRI},
	{"asri",	OP_ADD_ASRI},
	{"rori",	OP_ADD_RORI},
	{"shli",	OP_ADD_SHLI},
	{"mini",	OP_ADD_MINI},
	{"maxi",	OP_ADD_MAXI},
	{"andi",	OP_ADD_ANDI},
	{"ori",		OP_ADD_ORI},
	{"xori",	OP_ADD_XORI},
	{"noti",	OP_ADD_NOTI},
	{"clzi",	OP_ADD_CLZI},
	{"v8addsi",	OP_ADD_V8ADDSI},
	{"v8subsi",	OP_ADD_V8SUBSI},
	{"fmuli",	OP_MUL_FMULI},
	{"mul24i",	OP_MUL_MUL24I},
	{"v8muldi",	OP_MUL_V8MULDI},
	{"v8mini",	OP_MUL_V8MINI},
	{"v8maxi",	OP_MUL_V8MAX},

	{"v8asrotr5",	OP_MUL_V8ADDS_ROTR5},
	{"v8asrot1",	OP_MUL_V8ADDS_ROT1},
	{"v8asrot2",	OP_MUL_V8ADDS_ROT2},
	{"v8asrot3",	OP_MUL_V8ADDS_ROT3},
	{"v8asrot4",	OP_MUL_V8ADDS_ROT4},
	{"v8asrot5",	OP_MUL_V8ADDS_ROT5},
	{"v8asrot6",	OP_MUL_V8ADDS_ROT6},
	{"v8asrot7",	OP_MUL_V8ADDS_ROT7},
	{"v8asrot8",	OP_MUL_V8ADDS_ROT8},
	{"v8asrot9",	OP_MUL_V8ADDS_ROT9},
	{"v8asrot10",	OP_MUL_V8ADDS_ROT10},
	{"v8asrot11",	OP_MUL_V8ADDS_ROT11},
	{"v8asrot12",	OP_MUL_V8ADDS_ROT12},
	{"v8asrot13",	OP_MUL_V8ADDS_ROT13},
	{"v8asrot14",	OP_MUL_V8ADDS_ROT14},
	{"v8asrot15",	OP_MUL_V8ADDS_ROT15},

	{"b",		OP_BR_B},
	{"bl",		OP_BR_BL},

	{"li",		OP_IMM_LI},
	{"lis",		OP_IMM_LIS},
	{"liu",		OP_IMM_LIU},

	{"semup",	OP_SEM_SEMUP},
	{"semdn",	OP_SEM_SEMDN},

	{"brk",		OP_SIG_BREAK},
	{"ts",		OP_SIG_THRD_SWITCH},
	{"pe",		OP_SIG_PROG_END},
	{"wsb",		OP_SIG_WAIT_SB},
	{"usb",		OP_SIG_UNLOCK_SB},
	{"lts",		OP_SIG_LAST_THRD_SWITCH},
	{"cvr",		OP_SIG_COVERAGE},
	{"clr",		OP_SIG_COLOUR},
	{"clrpe",	OP_SIG_COLOUR_PROG_END},
	{"ldtmu0",	OP_SIG_LD_TMU0},
	{"ldtmu1",	OP_SIG_LD_TMU1},
	{"lda",		OP_SIG_LD_ALPHA},

	{"sf",		OP_FLAGS_SF},
	{"pm",		OP_FLAGS_PM},
};

struct reg {
	enum reg_file			rf;
	int				num;
};

struct op {
	enum op_code		code[2];
	enum cc			cc[2];
	struct reg		dst[2];
	struct reg		src[4];
	char			*src_label;
};

struct instr {
	int				pc;
	unsigned int			lo;
	unsigned int			hi;

	enum op_code			sig;

	char				sf;
	char				pm;
	char				ws;
	char				unpack;
	char				pack;
	char				rel_br;
	char				reg_br;

	struct op			op;

	const char			*buf;
	int				curr_token;
	char				**labels;
	int				num_labels;
};

#define ENC_ALU_MUL_1_POS		0
#define ENC_ALU_MUL_0_POS		3
#define ENC_ALU_ADD_1_POS		6
#define ENC_ALU_ADD_0_POS		9
#define ENC_ALU_RADDR_B_POS		12
#define ENC_ALU_RADDR_A_POS		18
#define ENC_ALU_OP_ADD_POS		24
#define ENC_ALU_OP_MUL_POS		29
#define ENC_ALU_MUL_1_BITS		3
#define ENC_ALU_MUL_0_BITS		3
#define ENC_ALU_ADD_1_BITS		3
#define ENC_ALU_ADD_0_BITS		3
#define ENC_ALU_RADDR_B_BITS		6
#define ENC_ALU_RADDR_A_BITS		6
#define ENC_ALU_OP_ADD_BITS		5
#define ENC_ALU_OP_MUL_BITS		3

#define ENC_WADDR_MUL_POS		0
#define ENC_WADDR_ADD_POS		6
#define ENC_WS_POS			12
#define ENC_SF_POS			13
#define ENC_COND_MUL_POS		14
#define ENC_COND_ADD_POS		17
#define ENC_PACK_POS			20
#define ENC_PM_POS			24
#define ENC_UNPACK_POS			25
#define ENC_SIG_POS			28
#define ENC_WADDR_MUL_BITS		6
#define ENC_WADDR_ADD_BITS		6
#define ENC_WS_BITS			1
#define ENC_SF_BITS			1
#define ENC_COND_MUL_BITS		3
#define ENC_COND_ADD_BITS		3
#define ENC_PACK_BITS			4
#define ENC_PM_BITS			1
#define ENC_UNPACK_BITS			3
#define ENC_SIG_BITS			4

#define ENC_BR_RADDR_A_POS		13
#define ENC_BR_REG_POS			18
#define ENC_BR_REL_POS			19
#define ENC_BR_COND_POS			20
#define ENC_BR_RADDR_A_BITS		5
#define ENC_BR_REG_BITS			1
#define ENC_BR_REL_BITS			1
#define ENC_BR_COND_BITS		4

static
int encode_cond(enum cc code)
{
	switch (code) {
	case CC_NEVER:			return 0;
	case CC_ALWAYS:			return 1;
	case CC_Z:			return 2;
	case CC_NZ:			return 3;
	case CC_N:			return 4;
	case CC_NN:			return 5;
	case CC_C:			return 6;
	case CC_NC:			return 7;
	default:			return -EINVAL;
	}
}

static
int encode_cond_br(enum cc code)
{
	switch (code) {
	case CC_ALL_Z:			return 0;
	case CC_ALL_NZ:			return 1;
	case CC_Z:			return 2;
	case CC_NZ:			return 3;
	case CC_ALL_N:			return 4;
	case CC_ALL_NN:			return 5;
	case CC_N:			return 6;
	case CC_NN:			return 7;
	case CC_ALL_C:			return 8;
	case CC_ALL_NC:			return 9;
	case CC_C:			return 10;
	case CC_NC:			return 11;
	case CC_ALWAYS:			return 15;
	default:			return -EINVAL;
	}
}

static
int encode_sig(enum op_code sig)
{
	switch (sig) {
	case OP_SIG_BREAK:		return 0;
	case OP_SIG_NONE:		return 1;
	case OP_SIG_THRD_SWITCH:	return 2;
	case OP_SIG_PROG_END:		return 3;
	case OP_SIG_WAIT_SB:		return 4;
	case OP_SIG_UNLOCK_SB:		return 5;
	case OP_SIG_LAST_THRD_SWITCH:	return 6;
	case OP_SIG_COVERAGE:		return 7;
	case OP_SIG_COLOUR:		return 8;
	case OP_SIG_COLOUR_PROG_END:	return 9;
	case OP_SIG_LD_TMU0:		return 10;
	case OP_SIG_LD_TMU1:		return 11;
	case OP_SIG_LD_ALPHA:		return 12;
	case OP_SIG_SIMM:		return 13;
	case OP_SIG_LI:			return 14;
	case OP_SIG_BR:			return 15;
	default:			return -EINVAL;
	}
}

static
int encode_alu_op_mul(enum op_code code)
{
	switch (code) {
	case OP_NOP:			return 0;

	case OP_MUL_FMUL:		return 1;
	case OP_MUL_MUL24:		return 2;
	case OP_MUL_V8MULD:		return 3;
	case OP_MUL_V8MIN:		return 4;
	case OP_MUL_V8MAX:		return 5;

	case OP_MUL_FMULI:		return 1;
	case OP_MUL_MUL24I:		return 2;
	case OP_MUL_V8MULDI:		return 3;
	case OP_MUL_V8MINI:		return 4;
	case OP_MUL_V8MAXI:		return 5;
	
	case OP_MUL_V8ADDS_ROTR5:
	case OP_MUL_V8ADDS_ROT1:
	case OP_MUL_V8ADDS_ROT2:
	case OP_MUL_V8ADDS_ROT3:
	case OP_MUL_V8ADDS_ROT4:
	case OP_MUL_V8ADDS_ROT5:
	case OP_MUL_V8ADDS_ROT6:
	case OP_MUL_V8ADDS_ROT7:
	case OP_MUL_V8ADDS_ROT8:
	case OP_MUL_V8ADDS_ROT9:
	case OP_MUL_V8ADDS_ROT10:
	case OP_MUL_V8ADDS_ROT11:
	case OP_MUL_V8ADDS_ROT12:
	case OP_MUL_V8ADDS_ROT13:
	case OP_MUL_V8ADDS_ROT14:
	case OP_MUL_V8ADDS_ROT15:	return 6;
	default:			return -EINVAL;
	}
}

static
int encode_alu_op_add(enum op_code code)
{
	switch (code) {
	case OP_NOP:			return 0;

	case OP_ADD_FADD:		return 1;
	case OP_ADD_FSUB:		return 2;
	case OP_ADD_FMIN:		return 3;
	case OP_ADD_FMAX:		return 4;
	case OP_ADD_FMINABS:		return 5;
	case OP_ADD_FMAXABS:		return 6;
	case OP_ADD_FTOI:		return 7;
	case OP_ADD_ITOF:		return 8;
	case OP_ADD_ADD:		return 12;
	case OP_ADD_SUB:		return 13;
	case OP_ADD_SHR:		return 14;
	case OP_ADD_ASR:		return 15;
	case OP_ADD_ROR:		return 16;
	case OP_ADD_SHL:		return 17;
	case OP_ADD_MIN:		return 18;
	case OP_ADD_MAX:		return 19;
	case OP_ADD_AND:		return 20;
	case OP_ADD_OR:			return 21;
	case OP_ADD_XOR:		return 22;
	case OP_ADD_NOT:		return 23;
	case OP_ADD_CLZ:		return 24;
	case OP_ADD_V8ADDS:		return 30;
	case OP_ADD_V8SUBS:		return 31;

	case OP_ADD_FADDI:		return 1;
	case OP_ADD_FSUBI:		return 2;
	case OP_ADD_FMINI:		return 3;
	case OP_ADD_FMAXI:		return 4;
	case OP_ADD_FMINABSI:		return 5;
	case OP_ADD_FMAXABSI:		return 6;
	case OP_ADD_FTOII:		return 7;
	case OP_ADD_ITOFI:		return 8;
	case OP_ADD_ADDI:		return 12;
	case OP_ADD_SUBI:		return 13;
	case OP_ADD_SHRI:		return 14;
	case OP_ADD_ASRI:		return 15;
	case OP_ADD_RORI:		return 16;
	case OP_ADD_SHLI:		return 17;
	case OP_ADD_MINI:		return 18;
	case OP_ADD_MAXI:		return 19;
	case OP_ADD_ANDI:		return 20;
	case OP_ADD_ORI:		return 21;
	case OP_ADD_XORI:		return 22;
	case OP_ADD_NOTI:		return 23;
	case OP_ADD_CLZI:		return 24;
	case OP_ADD_V8ADDSI:		return 30;
	case OP_ADD_V8SUBSI:		return 31;
	default:			return -EINVAL;
	}
}

static
int is_hex_digit(int c)
{
	int i;
	static const char *hex = "0123456789abcdef";
	static const char *HEX = "0123456789ABCDEF";

	for (i = 0; i < 16; ++i)
		if (c == hex[i] || c == HEX[i])
			return 1;
	return 0;
}

static
int count_ones(uint64_t mask)
{
	int num;

	num = 0;
	while (mask) {
		if (mask & 1)
			++num;
		mask >>= 1;
	}
	return num;
}
#endif
