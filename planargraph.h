/********************************************************************
Copyright 2017 Hung Le

********************************************************************/

#pragma once
#include "stdafx.h"

typedef struct vertex vertex;
typedef struct arc arc;
typedef std::unordered_map<__int64, int> arc_map_type;
//struct planargraph;


struct arc {
	vertex *source;
	vertex *sink;
	arc *nextarc;	// the next arc in the clock-wise rotational system
	arc *prevarc;	// the previous arc in the clock-wise rotational system 
	arc *rev;		// the rever of the arc
	int name;
	int index;
	bool mark;
	int version;

	bool operator==(const arc &other);
	bool operator!=(const arc &other);
};

bool arc::operator==(const arc &other) {
	return name == other.name;
}

bool arc::operator!=(const arc &other) {
	return name != other.name;
}

struct vertex {
	int name;		// name is subject to change, for example, create a subgraph of the original graph, we should only change name
	int id;			// id is never changed
	int index;		// for indexing
	std::vector<arc*> arclist;

	bool operator==(const vertex &other);
	bool operator!=(const vertex &other);
};

bool vertex::operator==(const vertex &other) {
	return id == other.id;
}

bool vertex::operator!=(const vertex &other) {
	return id != other.id;
}

struct graph {
	vertex *vertices;	// the set of vertices
	arc *arcs;
	int n = 0;
	int m = 0;
	int current_version = 0;	// useful when modifying the graph by adding arcs, each modification resulting a version
	int num_version = 0;		// the current number of versions
	int graph_version = 0;

	static int const white = 0;	// color for graph traversal
	static int const gray = 1;
	static int const black = 2;

	static void print_arc(arc *a);
	void create_arc_indices();
	arc create_arc(int sourceindex, int sinkindex);	// add an arc with a source and a sink
	void update_arc(int arc_index, int sourceindex, int sinkindex); // update an arc with a new sourc and a new sink
	vertex null_vertex();
	arc null_arc();
	__int64 arc_to_int64(vertex *soure, vertex *sink);
	void reindex_arcs();
	void reset_arc_marks();
	void print();
};

struct planargraph : graph {
	int max_num_arcs;
	std::unordered_map<__int64, int> arc_map;
	planargraph();
	planargraph(int nv, std::vector<std::vector<int>> & embedding);
	planargraph(int n, int m);
	~planargraph();
	void init(int n, int m);
	void check_rotational_system();
	void reset();	// remove the set of added arcs, only keep arcs of version 0
	// useful when doing triangulation
	void init_arc_map();
	void add_arc_to_map(vertex *source, vertex *sink, int uv_arc_index);
	std::vector<std::vector<int>> get_embedding();
	void write_to_file(std::ofstream &out_stream);
};

planargraph::planargraph(){
	vertices = nullptr;
	arcs = nullptr;
	n = 0;
	m = 0;
}
planargraph::planargraph(int nv, int ma) {
	init(nv, ma);
}

void planargraph::init(int nv, int ma) {
	n = nv;
	m = ma;
	vertices = new vertex[nv];
	arcs = new arc[6 * n];
	max_num_arcs = 6 * n;
}

planargraph::planargraph(int nv, std::vector<std::vector<int>> & embedding) {
	n = nv;
	vertices = new vertex[nv];
	for (int i = 0; i < n; i++) {
		vertices[i].name = i;
		vertices[i].index = i;
		vertices[i].id = i;
	}
	// read the embedding
	for (std::vector<std::vector<int>>::iterator it = embedding.begin(); it != embedding.end(); ++it) {
		m += (int)(*it).size();
	}
	arcs = new arc[6*n];
	max_num_arcs = 6 * n;
	arc_map.reserve(6*n);
	for (int i = 0; i < m; i++) {
		arcs[i] = null_arc();	// initialize null to evry arc
	}
	int u = 0;
	vertex *u_vertex;
	vertex *v_vertex;
	int arc_index = 0;
	int uv_arc_index = -1;
	int vu_arc_index = -1;
	for (std::vector<std::vector<int>>::iterator it = embedding.begin(); it != embedding.end(); ++it) {
		std::vector<int> rotation_around_u = *it;
		u_vertex = &vertices[u];
//		printf("%d:\t", u);
		//printf("deg:%d\t", rotation_around_u.size());
		arc *prev_arc = &arcs[arc_index + rotation_around_u.size() - 1]; //  the last arc in the rotation system of u
		for (std::vector<int>::iterator arc_it = rotation_around_u.begin(); arc_it != rotation_around_u.end(); ++arc_it) {
//			printf("%d\t", *arc_it);
			v_vertex = &vertices[*arc_it];
			arcs[arc_index].source = u_vertex;
			arcs[arc_index].sink = v_vertex;
			arcs[arc_index].name = arc_index;
			arcs[arc_index].index = arc_index;
			// update prev arc of uv in the rotation around u
			uv_arc_index = arc_index;
			arcs[uv_arc_index].prevarc = prev_arc;
			prev_arc->nextarc = &arcs[uv_arc_index];
			// update the neighbor list of u
			vertices[u].arclist.push_back(&arcs[uv_arc_index]);
			// update prev arc and arc index
			prev_arc = &arcs[uv_arc_index];
			arc_index++;
			// update the rev pointers
			arc_map.insert(std::unordered_map<__int64, int>::value_type(arc_to_int64(u_vertex, v_vertex), uv_arc_index)); // put u->v arc to the map
			if (v_vertex->index < u_vertex->index) {	
				// update the reverse pointer of u->v and v->u
				vu_arc_index = arc_map.find(arc_to_int64(v_vertex, u_vertex))->second;	// get the index of v->u arc
				arcs[uv_arc_index].rev = &arcs[vu_arc_index];
				arcs[vu_arc_index].rev = &arcs[uv_arc_index];
			}
		}
		//printf("\n");
		u++;
	}
	//check_rotational_system();
}

planargraph::~planargraph() {
	//printf("destruct a planargraph\n");
	delete[] vertices;
	delete[] arcs;
}

void planargraph::reset() {
	int num_arcs = 0;
	for (int i = 0; i < n; i++) {
		while (vertices[i].arclist.back()->version != 0) {
			vertices[i].arclist.pop_back();
		}
		num_arcs += (int)vertices[i].arclist.size();
	}
	m = num_arcs;
	// no need to update rev
	arc *nextarc, *prevarc; 
	for (int i = 0; i < m; i++) {
		nextarc = arcs[i].nextarc;
		while (nextarc->version != 0) {
			nextarc = nextarc->nextarc;
		}
		arcs[i].nextarc = nextarc;
		prevarc = arcs[i].prevarc;
		while (prevarc->version != 0) {
			prevarc = prevarc->prevarc;
		}
		arcs[i].prevarc = prevarc;
	}

}

void planargraph::add_arc_to_map(vertex *source, vertex *sink, int uv_arc_index) {
	arc_map.insert(std::unordered_map<__int64, int>::value_type(arc_to_int64(source, sink), uv_arc_index)); // put u->v arc to the map
}
void planargraph::init_arc_map() {
	vertex *u_vertex, *v_vertex;
	for (int i = 0; i < m; i++) {
		u_vertex = arcs[i].source;
		v_vertex = arcs[i].sink;
		arc_map.insert(std::unordered_map<__int64, int>::value_type(arc_to_int64(u_vertex, v_vertex), i)); // put u->v arc to the map
	}
}

std::vector<std::vector<int>> planargraph::get_embedding() {
	std::vector<std::vector<int>> embedding_storage;
	for (int i = 0; i < n; i++) {
		std::vector<int> rot; 
		arc *arc_it = vertices[i].arclist.front();
		while (arc_it->mark != true) {
			//printf("%d\t", arc_it->sink->index);
			rot.push_back(arc_it->sink->index);
			arc_it->mark = true;
			arc_it = arc_it->nextarc;
		}
		embedding_storage.push_back(rot);
//		printf("\n");
	}
	for (int i = 0; i < m; i++) {
		arcs[i].mark = false;
	}
	return embedding_storage;
}

void planargraph::write_to_file(std::ofstream &out_stream) {
	if (n == 1) {
		out_stream << vertices[0].id << ":";
		out_stream << std::endl;
		return;
	}
	for (int i = 0; i < n; i++) {
		arc *arc_it = vertices[i].arclist.front();
		out_stream << vertices[i].id << ":";
		while (arc_it->mark != true) {
			out_stream << " " << arc_it->sink->id;
			arc_it->mark = true;
			arc_it = arc_it->nextarc;
		}
		out_stream << std::endl;
	}
	for (int i = 0; i < m; i++) {
		arcs[i].mark = false;
	}
}
void graph::create_arc_indices() {
	for (int i = 0; i <m; i++) {
		arcs[i].index = i;
	}
}

__int64 graph::arc_to_int64(vertex *source, vertex *sink) {
	return ((__int64)source->index << 32) + (__int64)sink->index;
}

arc graph::create_arc(int sourceindex, int sinkindex) {
	arc a {};
	a.source = &vertices[sourceindex];
	a.sink = &vertices[sinkindex];
	a.nextarc = nullptr;
	a.prevarc = nullptr;
	a.rev = nullptr;
	a.mark = false;
	a.version = 0;
	return a;
}

void graph::update_arc(int arc_index, int sourceindex, int sinkindex) {
	arcs[arc_index].source = &vertices[sourceindex];
	arcs[arc_index].sink = &vertices[sinkindex];
	arcs[arc_index].nextarc = nullptr;
	arcs[arc_index].prevarc = nullptr;
	arcs[arc_index].rev = nullptr;
	arcs[arc_index].mark = false;
	arcs[arc_index].version = 0;
	arcs[arc_index].index = arc_index;
}

vertex graph::null_vertex() {
	vertex v{};
	v.id = -1;
	return v;
}

arc graph::null_arc() {
	arc a{};
	a.source = nullptr;
	a.sink = nullptr;
	a.name = -1;
	a.nextarc = nullptr;
	a.prevarc = nullptr;
	a.rev = nullptr;
	a.mark = false;
	a.version = 0;
	return a;
}
void graph::reindex_arcs() {
	for (int i = 0; i < m; i++) {
		arcs[i].index = i;
	}
}
void graph::reset_arc_marks() {
	for (int i = 0; i < m; i++) {
		arcs[i].mark = false;
	}
}


void graph::print_arc(arc *a) {
	printf("%d->%d\n", a->source->index, a->sink->index);
}

void graph::print() {
	printf("*******************************************\n");
	printf("Adjacency list:\n");
	for (int i = 0; i < n; i++) {
		printf("(%d,%d):\t", i, vertices[i].id);
		for (std::vector<arc*>::iterator arc_it = vertices[i].arclist.begin(); arc_it != vertices[i].arclist.end(); ++arc_it) {
			printf("(%d,%d)\t", (*arc_it)->sink->index, (*arc_it)->sink->id);
		}
		printf("\n");
	}
	printf("*******************************************\n");
}

void planargraph::check_rotational_system() {
	printf("\nChecking the rotational system\n");
	if (n == 1) {
		printf("The graph is singleton\n");
		return;
	}
	printf("Forward check\n");
	for (int i = 0; i < n; i++) {
		printf("%d:\t", i);
		arc *arc_it = vertices[i].arclist.front();
		while (arc_it->mark != true) {
			printf("%d\t", arc_it->sink->index);
			arc_it->mark = true;
			arc_it = arc_it->nextarc;
		}
		printf("\n");
	}
	for (int i = 0; i < m; i++) {
		arcs[i].mark = false;
	}
	printf("Backward check\n");
	for (int i = 0; i < n; i++) {
		printf("%d:\t", i);
		arc *arc_it = vertices[i].arclist.front();
		while (arc_it->mark != true) {
			printf("%d\t", arc_it->sink->index);
			arc_it->mark = true;
			arc_it = arc_it->prevarc;
		}
		printf("\n");
	}
	for (int i = 0; i < m; i++) {
		arcs[i].mark = false;
	}
	printf("Reverse check\n");
	for (int i = 0; i < m; i++) {
		printf("#%d\t arc:(%d,%d) and its rev: (%d,%d)\n",i, arcs[i].source->index, arcs[i].sink->index, arcs[i].rev->source->index, arcs[i].rev->sink->index);
	}
	printf("Checking done!\n");
}
