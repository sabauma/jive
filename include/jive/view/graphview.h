/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_GRAPHVIEW_H
#define JIVE_VIEW_GRAPHVIEW_H

#include <jive/internal/metacontainers.h>
#include <jive/util/hash.h>

#include <jive/view/nodeview.h>
#include <jive/vsdg.h>
#include <jive/util/textcanvas.h>

typedef struct jive_graphview jive_graphview;
typedef struct jive_graphview_row jive_graphview_row;

struct jive_graphview_row {
	int width, height, pad_above, pad_below;
	int x, y;
};

static inline int ptr_hash(void * n) {return (intptr_t)n;}

JIVE_DECLARE_HASH_TYPE(jive_nodeview_map, jive_nodeview, jive_node *, node, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_nodeview_map, jive_nodeview, jive_node *, node, hash_chain);

JIVE_DECLARE_HASH_TYPE(jive_inputview_map, jive_inputview, jive::input *, input, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_inputview_map, jive_inputview, jive::input *, input, hash_chain);

JIVE_DECLARE_HASH_TYPE(jive_outputview_map, jive_outputview, jive::output *, output, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_outputview_map, jive_outputview, jive::output *, output, hash_chain);

struct jive_graphview {
	struct jive_graph * graph;
	struct jive_nodeview_map nodemap;
	struct jive_inputview_map inputmap; 
	struct jive_outputview_map outputmap; 
	int width, height;
	
	size_t nrows;
	jive_graphview_row * rows;
	
	struct jive_regionview * root_region;
	
	jive_textcanvas canvas;
};

jive_graphview *
jive_graphview_create(jive_graph * graph);

void
jive_graphview_destroy(jive_graphview * self);

jive_graphview_row *
jive_graphview_get_row(jive_graphview * self, size_t index);

void
jive_graphview_draw(jive_graphview * self);

#endif
