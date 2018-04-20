/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
#include "stdafx.h"

/*
 * An independent set solver for planar graphs which is basically a backtracking program
 * Global variables of the backtracking program must be carefully maintained to guarantee correctness
 */
struct planar_is_solver {
	planargraph *g;
	int branch_count = 0;
	int solution_size = 0;

	// current graph statistics
	int curr_num_active_vertices = 0;
	int *degs;	// deleted vertices have deg -1
	arc** sample_active_arcs;		// a sample arc for each vertex
	planar_is_solver(planargraph *arg_g) : g(arg_g) {
		curr_num_active_vertices = g->n;
		degs = new int[g->n];
		sample_active_arcs = new arc*[g->n];
		for (int i = 0; i < curr_num_active_vertices; i++) {
			degs[i] = g->vertices[i].arclist.size();
			sample_active_arcs[i] = g->vertices[i].arclist.front();
		}
	}

	void find_independent_set() {
		if (curr_num_active_vertices <= 1) {
			std::cout << "Finish " << branch_count << "-th branch" << std::endl;
			return;
		}
		std::vector<int> deg_2_vertices;
		int deg_min_vertex;
		int deg_min = g->n;
		int deg_max_vertex;
		int deg_max = 0;
		int deg_v;
		// we keep track of deactivate edges during branching
		std::stack<arc*> edge_stack;
		for (int v = 0; v < g->n; v++) {
			if (degs[v] < 0) continue;
			deg_v = degs[v];
			if (deg_v <= 1) {
				// TO-DO case deg_v = 1 is different from case deg_v = 0
				deactivate_vertex(v, edge_stack);
				solution_size++;
			}
			else if (deg_v == 2) {
				deg_2_vertices.push_back(v);
				arc *uv = sample_active_arcs[v]->rev;
				arc *wv = sample_active_arcs[v]->nextarc->rev;
				int u = uv->source->index;
				int w = wv->source->index;
				// contract v and w to u
				deactivate_vertex(v, edge_stack);
				if (degs[w] == 0) {
					deactivate_vertex(w, edge_stack);
				}
				else if (degs[u] == 0) {
					deactivate_vertex(u, edge_stack);
				}
				else {
					// TO-DO need to recursively process v after contraction
					// contract w to u means change the name of w to the name of u
					// only the rotational systems of w and u changed. 
					g->vertices[w].name = g->vertices[v].name;
					//degs[w] = -1; 
					// deactivate arcs of w that connect the same neighbors with u
					arc *ait = wv->nextarc;
					arc *wa = nullptr;
					while (ait->sink->index != v) {
						if (g->arc_map.find(g->arc_to_int64(uv->source, ait->sink)) != g->arc_map.end()) {
							deactivate_edge(ait);
							edge_stack.push(ait);
						}
						else if (wa == nullptr) {
							wa = ait;
						}
					}
					if (degs[w] == 0) {
						uv->prevarc->nextarc = uv->nextarc;
						uv->nextarc->prevarc = uv->prevarc;
					}
					else {
						arc *ux = uv->prevarc;
						arc *uy = uv->nextarc;
						arc *wb = wa->prevarc;
						ux->nextarc = wa;
						wa->prevarc = ux;
						uy->prevarc = wb;
						wb->nextarc = uy;
					}
					// mark w deactivated 
					// note that arcs of w may not be deactivated
					degs[w] = -1;
				}
			}
		}



		print();
	}

	void deactivate_vertex(int v, std::stack<arc*> &edge_stack) {
		// symbolically remove a vertex v from g
		if (degs[v] > 0) {
			deactivate_edge(sample_active_arcs[v]);
			edge_stack.push(sample_active_arcs[v]);
			int u = sample_active_arcs[v]->sink->index;
			arc *it_arc = sample_active_arcs[v]->nextarc;
			while (it_arc->sink->index != u) {
				deactivate_edge(it_arc);
				edge_stack.push(it_arc);
				it_arc = it_arc->nextarc;
			}
		}
		degs[v] = -1;
	}
	void reactivate_vertex(int v) {
		// retore vertex v back to g
	}

	// deactive an edge from the graph
	// need to update sample_active_arc
	void deactivate_edge(arc *uv) {
		int u = uv->source->index;
		int v = uv->sink->index;
		degs[u]--;
		degs[v]--;
		uv->mark = true;
		arc *prv = uv->prevarc;
		arc *nxt = uv->nextarc;
		prv->nextarc = nxt;
		nxt->prevarc = prv;
		sample_active_arcs[u] = prv;

		arc *rev = uv->rev;
		rev->mark = true;
		prv = rev->prevarc;
		nxt = rev->nextarc;
		prv->nextarc = nxt;
		nxt->prevarc = prv;
		sample_active_arcs[v] = prv;
	}
	// reactive an edge from the graph
	void reactivate_edge(arc *uv) {

	}
	~planar_is_solver() {
		delete[] degs;
		delete[] sample_active_arcs;
	}

	void print() {
		printf("*******************************************\n");
		printf("Adjacency list:\n");
		for (int i = 0; i < g->n; i++) {
			if (degs[i] < 0) continue;
			printf("(%d,%d):\t", i, g->vertices[i].id);
			for (std::vector<arc*>::iterator arc_it = g->vertices[i].arclist.begin(); arc_it != g->vertices[i].arclist.end(); ++arc_it) {
				if ((*arc_it)->mark) continue;	// the arc is deactive
				printf("(%d,%d)\t", (*arc_it)->sink->index, (*arc_it)->sink->id);
			}
			printf("\n");
		}
		printf("*******************************************\n");
	}

};
