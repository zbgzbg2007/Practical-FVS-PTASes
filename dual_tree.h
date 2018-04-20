/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "bfs_tree.h"
#include "planargraph.h"
#include "planar_face_visitor.h"

/*
*	In constructing the dual tree of the bfs tree of a planar graph, we make use the fact that each arc of a planar graph
*	is associated with exactly one face. Thus, each non-tree arc of G can be used to identify a corresponding face of the dual tree
*	We assume that the input graph is triangulated
*/
struct  dual_tree : graph
{
	int *dual_vertex_index_to_arc_index;	// one arc is sufficient to indentify a face
	int *arc_index_to_dual_vertex_index;
	bfs_tree *primal_tree;
	graph *g;
	dual_tree( bfs_tree *arg_bfs_tree) : primal_tree(arg_bfs_tree) {
		g = primal_tree->g;
		//n = 2*primal_tree->g.n - 4;
		n = 2 * g->n - 4;
		vertices = new vertex[n];
		for (int i = 0; i < n; i++) {
			vertices[i].name = i;
			vertices[i].index = i;
			vertices[i].id = i;
		}
		m = 2 * (n - 1);
		arcs = new arc[m];
		dual_vertex_index_to_arc_index = new int[n];
		arc_index_to_dual_vertex_index = new int[g->m];
	}
	~dual_tree() {
		//printf("destruct the dual tree\n");
		delete[] arc_index_to_dual_vertex_index;
		delete[] dual_vertex_index_to_arc_index;
		delete[] vertices;
		delete[] arcs;
	}
	void print_dual_faces();
};


void dual_tree::print_dual_faces() {
	for (int i = 0; i < n; i++) {
		std::cout << "Face #" << i << std::endl;
		arc *uv = &g->arcs[dual_vertex_index_to_arc_index[i]];
		print_arc(uv);
		arc *current;
		for (current = uv->rev->prevarc; current != uv; current = current->rev->prevarc) {
			print_arc(current);
		}
	}
}

struct dual_tree_builder : face_traversal_visitor {
	dual_tree *dtree;
	int current_face_index = -1;
	dual_tree_builder(dual_tree *arg_dtree) : dtree(arg_dtree) {};
	
	void begin_traversal() {
		//printf("Begin face traversal\n");
	}
	void begin_face() {
		current_face_index++;
		//printf("Traverse a new face\n");
	}
	void next_vertex(vertex *v) {
		//printf("Process vertex %d\n", v->name);
	}

	void next_arc(arc *uv) {
		dtree->arc_index_to_dual_vertex_index[uv->index] = current_face_index;
		dtree->dual_vertex_index_to_arc_index[current_face_index] = uv->index;
		//printf("Process arc %d->%d\n", uv->source->name, uv->sink->name);
	}

	void end_face() {
		//printf("End traversing a face\n");
	}
	void end_traversal() {
		//printf("building the dual tree\n");
		int m = dtree->primal_tree->g->m;
		arc *primal_arcs = dtree->primal_tree->g->arcs;
		bool *primal_tree_arc_marker = dtree->primal_tree->tree_arc_marker;
		bool *arc_marker = new bool[m];
		int dual_u_index, dual_v_index;
		int current_arc_index = -1;
		int uv_arc_index = -1, vu_arc_index = -1;
		for (int i = 0; i < m; i++) arc_marker[i] = false;
		for (int i = 0; i < m; i++) {
			if ((!primal_tree_arc_marker[primal_arcs[i].index]) && (!primal_tree_arc_marker[primal_arcs[i].rev->index]) && (arc_marker[i])) {
				
				dual_u_index = dtree->arc_index_to_dual_vertex_index[primal_arcs[i].index];
				dual_v_index = dtree->arc_index_to_dual_vertex_index[primal_arcs[i].rev->index];
				// add u->v arc and v->u arc to the dual tree
				uv_arc_index = current_arc_index + 1;
				vu_arc_index = current_arc_index + 2;
				dtree->update_arc(uv_arc_index, dual_u_index, dual_v_index);
				dtree->update_arc(vu_arc_index, dual_v_index, dual_u_index);
				dtree->arcs[uv_arc_index].rev = &dtree->arcs[vu_arc_index];
				dtree->arcs[vu_arc_index].rev = &dtree->arcs[uv_arc_index];
				dtree->vertices[dual_u_index].arclist.push_back(&dtree->arcs[uv_arc_index]);
				dtree->vertices[dual_v_index].arclist.push_back(&dtree->arcs[vu_arc_index]);
				// we use nextarc pointer to point to the corresponding primal arc
				dtree->arcs[uv_arc_index].nextarc = &primal_arcs[i];
				dtree->arcs[vu_arc_index].nextarc = primal_arcs[i].rev;
				current_arc_index += 2;
			}
			arc_marker[i] = true;
			arc_marker[primal_arcs[i].rev->index] = true;
		}
		delete[] arc_marker;
	}

};
