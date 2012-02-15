#include <locale.h>
#include <assert.h>
#include <stdint.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/comparison/bitsgreater.h>

int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){bits32, bits32});
	jive_node_reserve(top);

	jive_output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive_output * c2 = jive_bitconstant_signed(graph, 32, INT32_MAX);
	jive_output * c3 = jive_bitconstant_signed(graph, 32, INT32_MIN);

	jive_output * sgreater0 = jive_bitsgreater(top->outputs[0], top->outputs[1]);
	jive_output * sgreater1 = jive_bitsgreater(c0, c1);
	jive_output * sgreater2 = jive_bitsgreater(c1, c0);
	jive_output * sgreater3 = jive_bitsgreater(top->outputs[0], c2);
	jive_output * sgreater4 = jive_bitsgreater(c3, top->outputs[1]);

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	jive_node * bottom = jive_node_create(graph->root_region,
		5, (const jive_type *[]){ctype, ctype, ctype, ctype, ctype},
		(jive_output *[]){sgreater0, sgreater1, sgreater2, sgreater3, sgreater4}, 0, NULL);
	jive_node_reserve(bottom);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(bottom->inputs[0]->origin->node, &JIVE_BITSGREATER_NODE));
	assert(jive_node_isinstance(bottom->inputs[1]->origin->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(bottom->inputs[2]->origin->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(bottom->inputs[3]->origin->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(bottom->inputs[4]->origin->node, &JIVE_CONTROL_FALSE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}