/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
struct face_traversal_visitor {

	virtual void begin_traversal() {};
	virtual void begin_face() {};
	virtual void next_vertex(vertex *v) {};
	virtual void next_arc(arc *uv) {};
	virtual void end_face() {};
	virtual void end_traversal() {};
};


void planar_face_traversal(planargraph *g, face_traversal_visitor &visitor) {
	visitor.begin_traversal();
	bool* arc_marker = new bool[g->m];
	for (int i = 0; i < g->m; i++) arc_marker[i] = false;
	arc *current_arc;
	int current_arc_index = 0;
	vertex *source;
	vertex *sink;
	bool is_new_face = false;
	int face_count = 0;
	for (int i = 0; i < g->m; i++) {
		current_arc = &g->arcs[i];
		current_arc_index = current_arc->index;
		if (!arc_marker[current_arc_index]) {
			visitor.begin_face();
			is_new_face = true;
			face_count++;
			//printf("face #%d\n", face_count);
		}
		// traverse the face incident to current_arc_index
		while (!arc_marker[current_arc_index]) {
			visitor.next_arc(current_arc);
			source = current_arc->source;
			sink = current_arc->sink;
			visitor.next_vertex(sink);
			// update current arc
			arc_marker[current_arc_index] = true;
			if(current_arc->rev->prevarc->version <= g->current_version){	// if the arc to be visited is not a new arc, i.e, arc added during traversal
				current_arc = current_arc->rev->prevarc;
				current_arc_index = current_arc->index;
			}
		}
		if (is_new_face) {
			visitor.end_face();
			is_new_face = false;
		}
	}
	visitor.end_traversal();
	//printf("#face = %d\n", face_count);
}
