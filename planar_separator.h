/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "low_radius_separator.h"
#include "planar_triangulator.h"

typedef std::vector<vertex*> vertex_container;
typedef std::vector<arc*> arc_container;

struct graph_components : separator_bfs_visitor {
	planargraph *g;
	int num_components;
	std::vector<vertex_container> vertices_of_components;
	std::vector<arc_container> arcs_of_components;
	vertex_container sources_of_components;
	// the current component source vertex
	vertex *current_source;
	int current_comp_index = -1;
	int *vertex_to_comp_id;

	graph_components(planargraph *arg_g) {
		init(arg_g);
	}
	graph_components() {
		vertex_to_comp_id = nullptr;
		num_components = 0;
	}
	~graph_components() {
		//printf("destruct graph component\n");
		delete[] vertex_to_comp_id;
		}

	void init(planargraph *arg_g) {
		g = arg_g;
		vertex_to_comp_id = new int[g->n];
		for (int i = 0; i < g->n; i++) {
			vertex_to_comp_id[i] = -1;
		}
		num_components = 0;
	}
	void new_component(vertex *u) {
		//printf("new component start at %d\n", u->index);
		current_source = u;
		current_comp_index++;
		sources_of_components.push_back(u);
		vertex_to_comp_id[u->index] = current_comp_index;
	}
	// the design of separator_bfs_visitor gurantee that both u and v are not in the separator
	void tree_arc(arc *uv) {
		vertex *u = uv->source;
		vertex *v = uv->sink;
		vertex_to_comp_id[v->index] = vertex_to_comp_id[u->index];
	}

	void finish_traversal() {
		num_components = current_comp_index + 1;
		//printf("#components = %d\n", num_components);
		for (int i = 0; i < num_components; i++) {
			vertices_of_components.push_back(vertex_container());
			arcs_of_components.push_back(arc_container());
		}
		int comp_id = -1;
		for (int i = 0; i < g->n; i++) {
			comp_id = vertex_to_comp_id[i];
			if (comp_id >= 0) {
				vertices_of_components[comp_id].push_back(&g->vertices[i]);
			}
		}
		int source_comp_id = -1;
		int sink_comp_id = -1;
		for (int i = 0; i < g->m; i++) {
			source_comp_id = vertex_to_comp_id[g->arcs[i].source->index];
			sink_comp_id = vertex_to_comp_id[g->arcs[i].sink->index];
			if (source_comp_id >= 0 && sink_comp_id >= 0) {
				arcs_of_components[source_comp_id].push_back(&g->arcs[i]);
			}
		}
		/*		for (int i = 0; i < num_components; i++) {
		printf("vertices of comp# %d\n", i);
		for (int j = 0; j < vertices_of_components[i].size(); j++) {
		printf("%d\t", vertices_of_components[i][j]->index);
		}
		printf("\n");
		printf("arcs of comp# %d\n", i);
		for (int j = 0; j < arcs_of_components[i].size(); j++) {
		printf("%d->%d\t",arcs_of_components [i][j]->source->index, arcs_of_components[i][j]->sink->index);
		}
		printf("\n");
		}*/
	};

	void discover_vertex(vertex *u) {}
	void examine_vertex(vertex *u) {}
	void non_tree_arc(arc *uv) {}
	void gray_sink(arc *uv) {}
	void black_sink(arc *uv) {}
	void finish_vertex(vertex *u) {}
	void finish_component(vertex *u) {}

};


void subplanargraph_by_contracting_l0_and_removing_l2(planargraph &g, planargraph &g_subgraph, int *levels, int l0, int l2) {
	int current_arc_index = 0;
	int current_vertex_index = 0;
	// update the set of vertices of the subgraph
	for (int i = 0; i < g.n; i++) {
		if (levels[i] <= l0) {
			// contracted vertices point to the corresponding vertex in the subgraph
			g.vertices[i].name = g_subgraph.n - 1;
		}
		else if (levels[i] >= l2){
			// removed vertices have name -1
			g.vertices[i].name = -1;
		}
		else {
			// recall id never change
			g_subgraph.vertices[current_vertex_index].id = g.vertices[i].id;
			// update the name of the corresponding vertex to update arcs later on
			g.vertices[i].name = current_vertex_index;
			g_subgraph.vertices[current_vertex_index].index = current_vertex_index;
			current_vertex_index++;
		}
	}
	
	g_subgraph.vertices[current_vertex_index].id = -2;	// the contracted vertex
	g_subgraph.vertices[current_vertex_index].index = current_vertex_index;
	// finish updating the vertex set
	// update arcs of the planar subgraph
	current_arc_index = 0;
	// we need to process an arc and its rev of the original graph g at the same time
	// we use mark field to check whether it is process or not
	vertex *u, *v;
	vertex *subgraph_u, *subgraph_v;// the corresponding version of u and v in its subgraphs
	arc *uv, *vu;
	for (int i = 0; i < g.m; i++) {
		uv = &g.arcs[i];
		if (g.arcs[i].mark == true) continue;
		u  = uv->source;
		v = uv->sink;
		if (levels[u->index] >= l2 || levels[v->index] >= l2) {
			// u and/or v are deleted
			g.arcs[i].name = -3;	
			g.arcs[i].rev->name = -3;
		}else if (levels[u->index] > l0 || levels[v->index] > l0) {
			subgraph_u = &g_subgraph.vertices[u->name];
			subgraph_v = &g_subgraph.vertices[v->name];
			vu = uv->rev;
			// the only possibe case that have parallel arc is subgraph_u or subgraph_v is the contracted vertex
			if (g_subgraph.arc_map.find(g.arc_to_int64(subgraph_u, subgraph_v)) == g_subgraph.arc_map.end()) {
				uv->name = current_arc_index;
				vu->name = current_arc_index + 1;
				// update arc u->v of the subgraph 
				g_subgraph.arcs[current_arc_index].source = subgraph_u;
				g_subgraph.arcs[current_arc_index].sink = subgraph_v;
				g_subgraph.arcs[current_arc_index].version = 0;
				g_subgraph.arcs[current_arc_index].mark = false;
				g_subgraph.arcs[current_arc_index].index = current_arc_index;
				g_subgraph.arcs[current_arc_index].name = i;
				subgraph_u->arclist.push_back(&g_subgraph.arcs[current_arc_index]);
				g.arcs[i].name = current_arc_index;
				current_arc_index++;
				// update arc v->u of the subgraph 
				g_subgraph.arcs[current_arc_index].source = subgraph_v;
				g_subgraph.arcs[current_arc_index].sink = subgraph_u;
				g_subgraph.arcs[current_arc_index].version = 0;
				g_subgraph.arcs[current_arc_index].mark = false;
				g_subgraph.arcs[current_arc_index].index = current_arc_index;
				g_subgraph.arcs[current_arc_index].name = vu->index;
				subgraph_v->arclist.push_back(&g_subgraph.arcs[current_arc_index]);
				g.arcs[i].rev->name = current_arc_index;
				// update the rev pointer of u->v and v->u
				g_subgraph.arcs[current_arc_index].rev = &g_subgraph.arcs[current_arc_index - 1];
				g_subgraph.arcs[current_arc_index - 1].rev = &g_subgraph.arcs[current_arc_index];
				current_arc_index++;
				// u or v in the subgraph is the contracted vertex
				if (subgraph_u->index == g_subgraph.n - 1 || subgraph_v->index == g_subgraph.n - 1) {
					g_subgraph.add_arc_to_map(subgraph_u, subgraph_v, current_arc_index - 2);
					g_subgraph.add_arc_to_map(subgraph_v, subgraph_u, current_arc_index - 1);
				}
			}
			else {
				g.arcs[i].name = -3;	// we mark this arc differently because it is deleted
				g.arcs[i].rev->name = -3;
			}
		}
		else {
			// uv is a contracted edge
			g.arcs[i].name = -2;
			g.arcs[i].rev->name = -2;
		}
		uv->mark = true;
		uv->rev->mark = true;
	}
	for (int i = 0; i < g.m; i++) g.arcs[i].mark = false;
	g_subgraph.m = current_arc_index;

	// update the rotational system
	arc *nextarc, *prevarc;
	for (int i = 0; i < g_subgraph.m; i++) {
		nextarc = g.arcs[g_subgraph.arcs[i].name].nextarc;
		while (nextarc->name <= - 2){
			if (nextarc->name == -3) {
				// the arc is deleted
				// jump to the next arc in the same rotational system around the soure
				nextarc = nextarc->nextarc;
			}
			else {
				// the arc is contracted
				// jump to the next arc in the same face
				nextarc = nextarc->rev->prevarc;
			}
			
		}
		g_subgraph.arcs[i].nextarc = &g_subgraph.arcs[nextarc->name];
		prevarc = g.arcs[g_subgraph.arcs[i].name].prevarc;
		while (prevarc->name <= -2) {
			if (prevarc->name == -3) {
				prevarc = prevarc->prevarc;
			}
			else {
				prevarc = prevarc->rev->prevarc;
			}
		}
		g_subgraph.arcs[i].prevarc = &g_subgraph.arcs[prevarc->name];
	}

	//printf("#arcs = %d\n", g_subgraph.m);
	//g_subgraph.print();
	//g_subgraph.check_rotational_system();
}
void find_separator(planargraph &g, std::vector<int> &separator_container) {
	bfs_tree primal_bfs_tree(&g, &g.vertices[0]);
	bfs(&g.vertices[0], g, primal_bfs_tree);
	int sqrt_n = ((int)sqrt(g.n));
	// Find a median level i such that L[0] + ... + L[i-1] <= n/2 and L[0]+...+ L[i] > n/2
	// Here, L[i] is the number of vertices at level i
	int i = -1;
	int *L = new int[g.n];
	for (int i = 0; i < g.n; i++) {
		L[i] = 0;
	}
	int maxLevel = 0;
	for (int i = 0; i < g.n; i++) {
		L[primal_bfs_tree.levels[i]] ++;
		maxLevel = (maxLevel < primal_bfs_tree.levels[i]) ? primal_bfs_tree.levels[i] : maxLevel;
	}
	int Li = 0;	// Li = L[0] +... + L[i]
	int half_n = g.n / 2;
	int med_level = 0;
	for (int i = 0; i <= maxLevel; i++) {
		if (Li + L[i] <= half_n) {
			Li += L[i];
			med_level++;
		}
		else {
			break;
		}
	}
	//std::cout << "Median level: " << med_level << std::endl;
	if (L[med_level] <= 2 * sqrt_n) {
		// found a separator
		//printf("middle layer is a good separator\n");
		for (int i = 0; i < g.n; i++) {
			if (primal_bfs_tree.levels[i] == med_level) {
				separator_container.push_back(i);
			}
		}
		return;
	}
	if (maxLevel <= (int)sqrt(2 * g.n)) {
		//printf("the input graph has small diameter\n");
		planar_triangulate(&g);
		find_low_radius_separator(&g, &g.vertices[0], separator_container);
		g.reset();
		return;
	}
	int k = Li + L[med_level];
	int two_sqrt_k = (int)(2 * sqrt(k));
	int two_sqrt_n_k = (int)(2 * sqrt(g.n - k));
	int l0 = med_level;
	while (l0 >= 0) {
		if (L[0] + 2 * (med_level - l0) <= two_sqrt_k) break;
		l0--;
	}
	int l2 = med_level + 1;
	while (l2 <= maxLevel) {
		if (L[l2] + 2 * (l2 - med_level - 1) <= two_sqrt_n_k) break;
		l2--;
	}
	//std::cout << "l0 = " << l0 << "\t l2= " << l2 << std::endl;
	// push vertices of l0 and l2 to the separator
	//std::vector<int> temp_sep;
	for (int i = 0; i <= g.n; i++) {
		if (primal_bfs_tree.levels[i] == l0 || primal_bfs_tree.levels[i] == l2) {
			separator_container.push_back(i);
	//		temp_sep.push_back(i);
		}
	}
	int middle_part_size = 0;
	for (int i = l0 + 1; i < l2; i++) {
		middle_part_size += L[i];
	}
	if (middle_part_size <= (2 * g.n) / 3) {
		//printf("l0 + l2 is a good separator\n");
		return;
	}
	int L02 = 0;	// the # of vertices of level <= l0 and >= l2
	for (int i = 0; i <= l0; i++) {
		L02 += L[i];
	}
	//printf("L02 = %d\n", L02);
	for (int i = l2; i <= maxLevel; i++) {
		L02 += L[i];
	}
	//printf("L0 = %d\n", L02);
	// contract vertices of l1 into a new vertex and delete vertices of l2 
	planargraph contracted_graph((g.n - L02)+1, 6 *((g.n - L02) + 1)-12);
	subplanargraph_by_contracting_l0_and_removing_l2(g, contracted_graph, primal_bfs_tree.levels, l0, l2);
	planar_triangulate(&contracted_graph);
	std::vector<int> middle_separator;
	find_low_radius_separator(&contracted_graph, &contracted_graph.vertices[contracted_graph.n-1], middle_separator);
	int vertex_id = -1;
	for (int i = 0; i < middle_separator.size(); i++) {
		vertex_id = contracted_graph.vertices[middle_separator[i]].id;
		// the contracted vertex has negative id
		if(vertex_id  >= 0) {
			separator_container.push_back(contracted_graph.vertices[middle_separator[i]].id);
		}
	}
	//printf("separator by 3 sets\n");
}
