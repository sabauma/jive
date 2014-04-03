/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <setjmp.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static void jump(void * where, const char * msg)
{
	jmp_buf * buffer=(jmp_buf *)where;
	longjmp(*buffer, 1);
}

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	JIVE_DECLARE_TYPE(type);
	JIVE_DECLARE_VALUE_TYPE(value_type);
	const jive_type * tmparray0[] = {type};
	
	jive_node * n1 = jive_node_create(region,
		0, NULL, NULL,
		1, tmparray0);
	
	bool error_handler_called = false;
	
	jmp_buf buffer;
	if (setjmp(buffer) == 0) {
		jive_set_fatal_error_handler(ctx, jump, &buffer);
		const jive_type * tmparray1[] = {value_type};
		jive_output * tmparray2[] = {n1->outputs[0]};
		jive_node_create(region,
			1, tmparray1, tmparray2,
			0, 0);
	} else {
		error_handler_called = true;
	}
	
	assert(error_handler_called);
	
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-typemismatch", test_main);
