/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_ADDRESS_TYPE(addrtype);

	jive_lambda * lambda = jive_lambda_begin(graph, 1, &addrtype, (const char *[]){"x"});
	jive_output * fct = jive_lambda_end(lambda, 1, &addrtype, lambda->arguments);

	const jive_type * fcttype = jive_output_get_type(fct);
	jive_node * bottom = jive_node_create(graph->root_region,
		1, &fcttype, &fct,
		0, NULL);
	(void) bottom;

	jive_view(graph, stdout);

	jive_lambda_node_address_transform((const jive_lambda_node *)fct->node, 32);

	jive_view(graph, stdout);

	assert(jive_node_isinstance(bottom->inputs[0]->origin->node, &JIVE_BITSTRING_TO_ADDRESS_NODE));

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-lambda_transform", test_main);