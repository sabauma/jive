#include <jive/regalloc/xpoint-private.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-variable.h>

jive_xpoint *
jive_xpoint_create(jive_shaped_node * shaped_node, jive_shaped_ssavar * shaped_ssavar)
{
	jive_context * context = shaped_node->shaped_graph->context;
	
	jive_xpoint * xpoint = jive_context_malloc(context, sizeof(*xpoint));
	xpoint->shaped_node = shaped_node;
	xpoint->shaped_ssavar = shaped_ssavar;
	xpoint->before_count = 0;
	xpoint->cross_count = 0;
	xpoint->after_count = 0;
	
	jive_ssavar_xpoint_hash_insert(&shaped_node->ssavar_xpoints, xpoint);
	jive_node_xpoint_hash_insert(&shaped_ssavar->node_xpoints, xpoint);
	
	return xpoint;
}

void
jive_xpoint_destroy(jive_xpoint * xpoint)
{
	jive_shaped_node * shaped_node = xpoint->shaped_node;
	jive_shaped_ssavar * shaped_ssavar = xpoint->shaped_ssavar;
	jive_context * context = shaped_node->shaped_graph->context;
	
	jive_ssavar_xpoint_hash_remove(&shaped_node->ssavar_xpoints, xpoint);
	jive_node_xpoint_hash_remove(&shaped_ssavar->node_xpoints, xpoint);
	
	jive_context_free(context, xpoint);
}

