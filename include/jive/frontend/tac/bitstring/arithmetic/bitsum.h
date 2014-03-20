/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_ARITHMETIC_BITSUM_H
#define JIVE_FRONTEND_TAC_BITSTRING_ARITHMETIC_BITSUM_H

#include <jive/frontend/tac/three_address_code.h>

extern const jive_three_address_code_class JIVE_BITSUM_CODE;

typedef struct jive_bitsum_code jive_bitsum_code;

struct jive_bitsum_code {
	jive_three_address_code base;
};

static inline jive_bitsum_code *
jive_bitsum_code_cast(struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITSUM_CODE))
		return (jive_bitsum_code *) tac;
	else
		return 0;
}

static inline const jive_bitsum_code *
jive_bitsum_code_const_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITSUM_CODE))
		return (const jive_bitsum_code *) tac;
	else
		return 0;
}

struct jive_three_address_code *
jive_bitsum_code_create(struct jive_basic_block * basic_block,
	struct jive_three_address_code * summand1, struct jive_three_address_code * summand2);

#endif