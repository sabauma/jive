/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_REGISTERSET_H
#define JIVE_BACKEND_I386_REGISTERSET_H

#include <jive/arch/registers.h>

typedef enum {
	/* "top-level" register classes */
	jive_i386_gpr = 0,
	jive_i386_fp = 1,
	jive_i386_mmx = 2,
	jive_i386_sse = 3,
	jive_i386_flags = 8,
	
	/* gpr sub classes */
	jive_i386_gpr_byte = 16, /* registers that are byte-addressible */
	jive_i386_gpr_eax = 17,	
	jive_i386_gpr_ebx = 18,
	jive_i386_gpr_ecx = 19,	
	jive_i386_gpr_edx = 20,
	jive_i386_gpr_esi = 21,
	jive_i386_gpr_edi = 22,
	jive_i386_gpr_esp = 23,
	jive_i386_gpr_ebp = 24,

	/* fp sub classes */
	jive_i386_fp_st0 = 33,

	/* sse sub classes */
	jive_i386_sse_xmm0 = 41,
	jive_i386_sse_xmm1 = 42,
	jive_i386_sse_xmm2 = 43,
	jive_i386_sse_xmm3 = 44,
	jive_i386_sse_xmm4 = 45,
	jive_i386_sse_xmm5 = 46,
	jive_i386_sse_xmm6 = 47,
	jive_i386_sse_xmm7 = 48,
} jive_i386_regcls_index;

extern const struct jive_register_class jive_i386_regcls[];

#define jive_i386_cls_gpr (&jive_i386_regcls[jive_i386_gpr].base)
#define jive_i386_cls_fp (&jive_i386_regcls[jive_i386_fp].base)
#define jive_i386_cls_mmx (&jive_i386_regcls[jive_i386_mmx].base)
#define jive_i386_cls_sse (&jive_i386_regcls[jive_i386_sse].base)

/* gpr register classes */
#define jive_i386_cls_flags (&jive_i386_regcls[jive_i386_flags].base)
#define jive_i386_cls_byte (&jive_i386_regcls[jive_i386_gpr_byte].base)
#define jive_i386_cls_eax (&jive_i386_regcls[jive_i386_gpr_eax].base)
#define jive_i386_cls_ebx (&jive_i386_regcls[jive_i386_gpr_ebx].base)
#define jive_i386_cls_ecx (&jive_i386_regcls[jive_i386_gpr_ecx].base)
#define jive_i386_cls_edx (&jive_i386_regcls[jive_i386_gpr_edx].base)
#define jive_i386_cls_esi (&jive_i386_regcls[jive_i386_gpr_esi].base)
#define jive_i386_cls_edi (&jive_i386_regcls[jive_i386_gpr_edi].base)
#define jive_i386_cls_esp (&jive_i386_regcls[jive_i386_gpr_esp].base)
#define jive_i386_cls_ebp (&jive_i386_regcls[jive_i386_gpr_ebp].base)

/* fp register classes */
#define jive_i386_cls_st0 (&jive_i386_regcls[jive_i386_fp_st0].base)

/* sse register classes */
#define jive_i386_cls_xmm0 (&jive_i386_regcls[jive_i386_sse_xmm0].base)
#define jive_i386_cls_xmm1 (&jive_i386_regcls[jive_i386_sse_xmm1].base)
#define jive_i386_cls_xmm2 (&jive_i386_regcls[jive_i386_sse_xmm2].base)
#define jive_i386_cls_xmm3 (&jive_i386_regcls[jive_i386_sse_xmm3].base)
#define jive_i386_cls_xmm4 (&jive_i386_regcls[jive_i386_sse_xmm4].base)
#define jive_i386_cls_xmm5 (&jive_i386_regcls[jive_i386_sse_xmm5].base)
#define jive_i386_cls_xmm6 (&jive_i386_regcls[jive_i386_sse_xmm6].base)
#define jive_i386_cls_xmm7 (&jive_i386_regcls[jive_i386_sse_xmm7].base)

typedef enum {
	/* gpr registers */
	jive_i386_cc = 0,
	jive_i386_eax = 1,
	jive_i386_ecx = 2,
	jive_i386_ebx = 3,
	jive_i386_edx = 4,
	jive_i386_esi = 5,
	jive_i386_edi = 6,
	jive_i386_ebp = 7,
	jive_i386_esp = 8,

	/* fp registers */
	jive_i386_st0 = 9,

	/* sse registers */
	jive_i386_xmm0 = 10,
	jive_i386_xmm1 = 11,
	jive_i386_xmm2 = 12,
	jive_i386_xmm3 = 13,
	jive_i386_xmm4 = 14,
	jive_i386_xmm5 = 15,
	jive_i386_xmm6 = 16,
	jive_i386_xmm7 = 17,
} jive_i386_reg_index;

extern const jive_register_name jive_i386_regs [];

#endif
