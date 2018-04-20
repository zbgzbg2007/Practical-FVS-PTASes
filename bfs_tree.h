/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
#include "bfs_visitor.h"

struct bfs_tree : bfs_visitor {

	graph *g;
	int*  levels;
	bool* tree_arc_marker;
	vertex *root;

	bfs_tree(graph *arg_g, vertex *arg_root) : g(arg_g), root(arg_root) {
		levels = new int[g->n];
		for (int i = 0; i < g->n; i++) levels[i] = -1;
		levels[root->index] = 0;
		tree_arc_marker = new bool[g->m];
		for (int i = 0; i < g->m; i++) tree_arc_marker[i] = false;
	};
	~bfs_tree() {
		//printf("destruct bfs primal tree\n");
		delete[] levels;
		delete[] tree_arc_marker;
	}

	void discover_vertex(vertex *u) {}

	void examine_vertex(vertex *u) {}
	void tree_arc(arc *uv) {
		tree_arc_marker[uv->index] = true;
		levels[uv->sink->index] = levels[uv->source->index] + 1;
		//printf("Tree arc %d->%d\n", uv->source->index, uv->sink->index);
	}

	void non_tree_arc(arc *uv) {}
	void gray_sink(arc *uv) {}
	void black_sink(arc *uv) {}
	void finish_vertex(vertex *u) {}

	void print() {
		std::cout << "Levels: \n" << std::endl;
		for (int i = 0; i < g->n; i++) std::cout << "level[" << i << "] = " << levels[i] << std::endl;
		std::cout << "Tree arcs: \n" << std::endl;
		for (int i = 0; i < g->m; i++) {
			if (tree_arc_marker[i]) {
				g->print_arc(&g->arcs[i]);
			}
		}
	}

};
