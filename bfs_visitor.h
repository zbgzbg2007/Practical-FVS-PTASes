/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"

// we adopt visitor class definition of boost
// see http://www.boost.org/doc/libs/1_46_1/libs/graph/doc/BFSVisitor.html for more details
struct bfs_visitor {
	
	virtual void discover_vertex(vertex *u) {};
	virtual void examine_vertex(vertex *u) {};
	virtual void examine_arc(arc *uv) {};
	virtual void tree_arc(arc *uv) {};
	virtual void non_tree_arc(arc *uv) {};
	virtual void gray_sink(arc *uv) {};	// equivalent to gray_target of boost
	virtual void black_sink(arc *uv) {};
	virtual void finish_vertex(vertex *u) {};
};
struct separator_bfs_visitor {
	virtual void new_component(vertex *u) {};	// start visit a new component
	virtual void discover_vertex(vertex *u) {};
	virtual void examine_vertex(vertex *u) {};
	virtual void examine_arc(arc *uv) {};
	virtual void tree_arc(arc *uv) {};
	virtual void non_tree_arc(arc *uv) {};
	virtual void gray_sink(arc *uv) {};	// equivalent to gray_target of boost
	virtual void black_sink(arc *uv) {};
	virtual void finish_vertex(vertex *u) {};
	virtual void finish_component(vertex *u) {};
	virtual void finish_traversal() {};
};

void bfs(vertex *source, graph &g, bfs_visitor &vis) {
	int *color = new int[g.n];	
	//int white = 0, gray = 1, black = 2;
	// 0 = while, 1 = gray, 2 = black
	for (int i = 0; i < g.n; i++) {
		color[i] = g.white;
	}
	std::queue<vertex*> vqueue;
	vis.discover_vertex(source);
	vqueue.push(source);
	color[source->index] = g.gray;
	vertex *u;
	vertex *v;
	arc *uv;
	while (!vqueue.empty())
	{
		u = vqueue.front();
		vqueue.pop();
		if (color[u->index] == g.black) continue;
		vis.examine_vertex(u);
		for (std::vector<arc*>::iterator it = g.vertices[u->index].arclist.begin(); it != g.vertices[u->index].arclist.end(); ++it){
			uv = *it;
			v = uv->sink;
			vis.examine_arc(uv);
			if (color[v->index] == g.white) {
				vis.discover_vertex(v);
				vqueue.push(v);
				color[v->index] = g.gray;
				vis.tree_arc(uv);
			}
			else {
				if (color[v->index] == g.gray) {
					vis.gray_sink(uv);
				}
				else {	// color of v must be black
					vis.black_sink(uv);
				}
				vis.non_tree_arc(uv);
			}
		} // end examine neighbors of u
		color[u->index] = g.black;
		vis.finish_vertex(u);
	}
	delete[] color;
}

// bfs the graph g asssumed that vertices in the separator are removed
// use this bfs to count the number of components and its statistics after removing the separator
// and to find induced subgraphs of the components
void separtor_bfs(graph &g, separator_bfs_visitor &vis, std::vector<int> &separator){
	int *color = new int[g.n];
	bool *sep_marker = new bool[g.n];
	for (int i = 0; i < g.n; i++) {
		color[i] = g.white;
		sep_marker[i] = false;
	}
	for (int i = 0; i < separator.size(); i++) {
		sep_marker[separator[i]] = true;
	}
	vertex *source = nullptr;
	for (int i = 0; i < g.n; i++) {
		if (color[i] == g.white && (!sep_marker[i])) {
			source = &g.vertices[i];
			vis.new_component(source);
			std::queue<vertex*> vqueue;
			vis.discover_vertex(source);
			vqueue.push(source);
			color[source->index] = g.gray;
			vertex *u;
			vertex *v;
			arc *uv;
			while (!vqueue.empty())
			{
				u = vqueue.front();
				vqueue.pop();
				if (color[u->index] == g.black) continue;
				vis.examine_vertex(u);
				for (std::vector<arc*>::iterator it = g.vertices[u->index].arclist.begin(); it != g.vertices[u->index].arclist.end(); ++it) {
					uv = *it;
					v = uv->sink;
					if (sep_marker[v->index]) continue;
					vis.examine_arc(uv);
					if (color[v->index] == g.white) {
						vis.discover_vertex(v);
						vqueue.push(v);
						color[v->index] = g.gray;
						vis.tree_arc(uv);
					}
					else {
						if (color[v->index] == g.gray) {
							vis.gray_sink(uv);
						}
						else {	// color of v must be black
							vis.black_sink(uv);
						}
						vis.non_tree_arc(uv);
					}
				} // end examine neighbors of u
				color[u->index] = g.black;
				vis.finish_vertex(u);
			} // end while
			vis.finish_component(source);
		} // end if
	}// end for
	vis.finish_traversal();
	delete[] color;
	delete[] sep_marker;
}
