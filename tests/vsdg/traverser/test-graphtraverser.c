/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

void test_basic_traversal(jive_graph * graph, jive::node * n1, jive::node * n2)
{
	jive::node * tmp;
	
	{
		jive::topdown_traverser trav(graph->root());
		
		tmp = trav.next();
		assert(tmp==n1);
		tmp = trav.next();
		assert(tmp==n2);
		tmp = trav.next();
		assert(tmp == graph->root()->bottom());
		tmp = trav.next();
		assert(tmp==0);
	}
	
	{
		jive::bottomup_traverser trav(graph);
		tmp = trav.next();
		assert(tmp == graph->root()->bottom());
		tmp = trav.next();
		assert(tmp==n2);
		tmp = trav.next();
		assert(tmp==n1);
		tmp = trav.next();
		assert(tmp==0);
	}
}

void test_order_enforcement_traversal()
{
	jive_graph graph;
	
	jive_test_value_type type;
	jive::node * n1 = jive_test_node_create(graph.root(), {}, {}, {&type, &type});
	jive::node * n2 = jive_test_node_create(graph.root(), {&type}, {n1->output(0)}, {&type});
	jive::node * n3 = jive_test_node_create(graph.root(),
		{&type, &type}, {n2->output(0), n1->output(1)}, {&type});

	jive::node * tmp;
	
	{
		jive::topdown_traverser trav(graph.root());

		tmp = trav.next();
		assert(tmp == graph.root()->bottom());
		tmp = trav.next();
		assert(tmp==n1);
		tmp = trav.next();
		assert(tmp==n2);
		tmp = trav.next();
		assert(tmp==n3);
		tmp = trav.next();
		assert(tmp==0);
	}
	
	{
		jive::bottomup_traverser trav(&graph);

		tmp = trav.next();
		assert(tmp==n3);
		tmp = trav.next();
		assert(tmp==n2);
		tmp = trav.next();
		assert(tmp == graph.root()->bottom());
		tmp = trav.next();
		assert(tmp==n1);
		tmp = trav.next();
		assert(tmp==0);
	}
}

void test_traversal_insertion(jive_graph * graph, jive::node * n1, jive::node * n2)
{
	jive::node * node;
	
	jive::topdown_traverser trav(graph->root());
	
	jive_test_value_type type;
	node = trav.next();
	assert(node==n1);

	/* At this point, n1 has been visited, now create some nodes */

	jive::node * n3 = jive_test_node_create(graph->root(), {}, {}, {&type});
	jive::node * n4 = jive_test_node_create(graph->root(), {&type}, {n3->output(0)}, {});
	jive::node * n5 = jive_test_node_create(graph->root(), {&type}, {n2->output(0)}, {});

	/*
		The newly created nodes n3 and n4 will not be visited,
		as they were not created as descendants of unvisited
		nodes. n5 must be visited, as n2 has not been visited yet.
	*/
	
	bool visited_n2 = false, visited_n3 = false, visited_n4 = false, visited_n5 = false;
	for(;;) {
		node = trav.next();
		if (!node) break;
		if (node==n2) visited_n2 = true;
		if (node==n3) visited_n3 = true;
		if (node==n4) visited_n4 = true;
		if (node==n5) visited_n5 = true;
	}
	
	assert(visited_n2);
	assert(!visited_n3);
	assert(!visited_n4);
	assert(visited_n5);
	
	graph->prune();
}

static int test_main(void)
{
	jive_graph graph;
	jive_test_value_type type;
	jive::node * n1 = jive_test_node_create(graph.root(), {}, {}, {&type, &type});
	jive::node * n2 = jive_test_node_create(graph.root(),
		{&type, &type}, {n1->output(0), n1->output(1)}, {&type});

	graph.root()->bottom()->add_input(&type, n2->output(0));
	(void)n1;
	(void)n2;
	
	/*
		The &graph now looks like the following:
		
		╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴
		╷┏━━━━━━━━━━━━━━┓╵
		╷┃              ┃╵
		╷┠──────────────┨╵
		╷┃      n1      ┃╵
		╷┠──────┬───────┨╵
		╷┃ #0:X │ #1:X  ┃╵
		╷┗━━━┯━━┷━━━┯━━━┛╵
		╷    │      │    ╵
		╷    │      │    ╵
		╷┏━━━┷━━┯━━━┷━━━┓╵
		╷┃ #0:X │ #1:X  ┃╵
		╷┠──────┴───────┨╵
		╷┃      n2      ┃╵
		╷┠──────────────┨╵
		╷┃ #0:X         ┃╵
		╷┗━━━━━━━━━━━━━━┛╵
		╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╵
		
		Topdown traversal will encounter n1, n2, in this order.
	*/
	
	test_basic_traversal(&graph, n1, n2);
	test_basic_traversal(&graph, n1, n2);
	test_order_enforcement_traversal();
	
	test_traversal_insertion(&graph, n1, n2);
	
	test_basic_traversal(&graph, n1, n2);
	
	assert(!graph.has_active_traversers());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/traverser/test-graphtraverser", test_main);
