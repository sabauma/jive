/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>
#include <stdint.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/comparison/bitugreater.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	const jive_type * tmparray0[] = {bits32, bits32};
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray0);

	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 4);
	jive_output * c1 = jive_bitconstant_unsigned(graph, 32, 5);
	jive_output * c2 = jive_bitconstant_unsigned(graph, 32, (0xffffffffUL));
	jive_output * c3 = jive_bitconstant_unsigned(graph, 32, 0);

	jive_output * ugreater0 = jive_bitugreater(top->outputs[0], top->outputs[1]);
	jive_output * ugreater1 = jive_bitugreater(c0, c1);
	jive_output * ugreater2 = jive_bitugreater(c1, c0);
	jive_output * ugreater3 = jive_bitugreater(top->outputs[0], c2);
	jive_output * ugreater4 = jive_bitugreater(c3, top->outputs[1]);

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	const jive_type * tmparray1[] = {ctype, ctype, ctype, ctype, ctype};
	jive_output * tmparray2[] = {ugreater0, ugreater1, ugreater2, ugreater3, ugreater4};
	jive_node * bottom = jive_node_create(graph->root_region,
		5, tmparray1,
		tmparray2, 1, &bits32);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(bottom->inputs[0]->origin->node, &JIVE_BITUGREATER_NODE));
	assert(jive_node_isinstance(bottom->inputs[1]->origin->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(bottom->inputs[2]->origin->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(bottom->inputs[3]->origin->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(bottom->inputs[4]->origin->node, &JIVE_CONTROL_FALSE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreater", test_main);
