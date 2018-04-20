/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
// we adopt visitor class definition of boost
// see http://www.boost.org/doc/libs/1_65_1/libs/graph/doc/DFSVisitor.html for more details

struct dfs_visitor {

	virtual void discover_vertex(vertex *u) {};
	virtual void examine_arc(arc *uv) {};
	virtual void tree_arc(arc *uv) {};
	virtual void back_arc(arc *uv) {};
	virtual void forward_or_cross_arc(arc *uv) {};	
	virtual void finish_vertex(vertex *u) {};	// when all out-going arcs of u are examined
};

typedef std::vector<arc*>::iterator arc_iterator_type;
typedef std::pair<arc_iterator_type, arc_iterator_type> iterator_pair_type;
typedef std::pair<vertex *, iterator_pair_type> vertex_info;

void dfs(vertex *source, graph &g, dfs_visitor &vis) {
	int *color = new int[g.n];
	for (int i = 0; i < g.n; i++) {
		color[i] = g.white;
	}
	std::vector<vertex_info> C;
	vis.discover_vertex(source);
	C.push_back(std::make_pair(source, std::make_pair(g.vertices[source->index].arclist.begin(), g.vertices[source->index].arclist.end())));
	color[source->index] = g.gray;
	vertex *u;
	vertex *v;
	arc *uv;
	vertex_info u_info;
	while (!C.empty())
	{
		u_info = C.back();
		u = u_info.first;
		C.pop_back();
		arc_iterator_type it = u_info.second.first;
		arc_iterator_type arc_it_end = u_info.second.second;
		while (it != arc_it_end) {
			uv = *it;
			v = uv->sink;
			if (color[v->index] == g.white) {
				vis.tree_arc(uv);
				C.push_back(std::make_pair(u, std::make_pair(++it, arc_it_end)));
				u = v;
				color[u->index] = g.gray;
				vis.discover_vertex(u);
				it = g.vertices[u->index].arclist.begin();
				arc_it_end = g.vertices[u->index].arclist.end();
			}
			else {
				if (color[v->index] == g.gray) {
					vis.back_arc(uv);
				}
				else {
					vis.forward_or_cross_arc(uv);
				}
				++it;
			}		
		}
		color[u->index] = g.black;
		vis.finish_vertex(u);
	}
	delete[] color;
}
