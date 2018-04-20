/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
#include "stdafx.h"

using namespace std;
struct sample_dfs_visitor : dfs_visitor {
	void discover_vertex(vertex *u) {
		//printf("Encounter %d for the first time\n", u->id);
	}

	void examine_arc(arc *uv) {
		//printf("Visit arc %d->%d\n", uv->source->id, uv->sink->id);
	}

	void tree_arc(arc *uv) {
		//printf("Tree arc %d->%d\n", uv->source->id, uv->sink->id);
	}

	void back_arc(arc *uv) {
		//printf("Back arc %d->%d\n", uv->source->id, uv->sink->id);
	}

	void forward_or_cross_arc(arc *uv) {
		//printf("Forward or cross arc %d->%d\n", uv->source->id, uv->sink->id);
	}

	void finish_vertex(vertex *u) {
		//printf("All vertices reachable from %d are visited\n", u->id);
	}

};
struct sample_bfs_visitor : bfs_visitor {
	void discover_vertex(vertex *u) {
		//printf("Put %d to the queue\n", u->id);
	}

	void examine_vertex(vertex *u) {
		//printf("Pop %d from the queue\n", u->id);
	}
	void tree_arc(arc *uv) {
		//printf("Tree arc %d->%d\n", uv->source->id, uv->sink->id);
	}

	void non_tree_arc(arc *uv) {
		//printf("Non tree arc %d->%d when examining neighbors of %d\n", uv->source->id, uv->sink->id, uv->source->id);
	}

	void gray_sink(arc *uv) {
		//printf("Neighbor %d of %d is not yet visited and in queue\n", uv->source->id, uv->sink->id);
	}

	void black_sink(arc *uv) {
		//printf("Neighbor %d of %d is visited\n", uv->source->id, uv->sink->id);
	}
	void finish_vertex(vertex *u) {
		//printf("All neighbor of %d is put in queue or visited\n", u->id);
	}

};
struct sample_face_visitor : face_traversal_visitor {
	void begin_traversal() {
		//printf("Begin face traversal\n");
	}
	void begin_face() {
		//printf("Traverse a new face\n");
	}
	void next_vertex(vertex *v) {
		//printf("Process vertex %d\n", v->name);
	}

	void next_arc(arc *uv) {
		//printf("Process arc %d->%d\n", uv->source->name, uv->sink->name);
	}

	void end_face() {
		//printf("End traversing a face\n");
	}
	void end_traversal() {
		//printf("End face traversal\n");
	}

};


// this planar graph  is smallest that allows one to test all four cases of low radius separator algorithm
vector<vector<int>> create_special_embedding() {
	vector<vector<int>> embedding_storage;
	vector<int> rot0;
	rot0.push_back(2); rot0.push_back(3); rot0.push_back(1);
	embedding_storage.push_back(rot0);

	vector<int> rot1;
	rot1.push_back(0); rot1.push_back(3); rot1.push_back(4); rot1.push_back(2);
	embedding_storage.push_back(rot1);

	vector<int> rot2;
	rot2.push_back(1); rot2.push_back(4); rot2.push_back(3); rot2.push_back(0);
	embedding_storage.push_back(rot2);

	vector<int> rot3;
	rot3.push_back(0); rot3.push_back(2); rot3.push_back(4); rot3.push_back(1);
	embedding_storage.push_back(rot3);

	vector<int> rot4;
	rot4.push_back(3); rot4.push_back(2); rot4.push_back(1);
	embedding_storage.push_back(rot4);
	return embedding_storage;

}

vector<vector<int>> create_sample_3x3_grid_embedding() {
	vector<vector<int>> embedding_storage;
	vector<int> rot0;
	rot0.push_back(1); rot0.push_back(3);
	embedding_storage.push_back(rot0);

	vector<int> rot1;
	rot1.push_back(2); rot1.push_back(4); rot1.push_back(0);
	embedding_storage.push_back(rot1);

	vector<int> rot2;
	rot2.push_back(5); rot2.push_back(1);
	embedding_storage.push_back(rot2);

	vector<int> rot3;
	rot3.push_back(0); rot3.push_back(4); rot3.push_back(6);
	embedding_storage.push_back(rot3);

	vector<int> rot4;
	rot4.push_back(1); rot4.push_back(5); rot4.push_back(7); rot4.push_back(3);
	embedding_storage.push_back(rot4);

	vector<int> rot5;
	rot5.push_back(2); rot5.push_back(8); rot5.push_back(4);
	embedding_storage.push_back(rot5);

	vector<int> rot6;
	rot6.push_back(3); rot6.push_back(7);
	embedding_storage.push_back(rot6);

	vector<int> rot7;
	rot7.push_back(4); rot7.push_back(8); rot7.push_back(6);
	embedding_storage.push_back(rot7);

	vector<int> rot8;
	rot8.push_back(5); rot8.push_back(7);
	embedding_storage.push_back(rot8);

	return embedding_storage;
}

vector<vector<int>> create_sample_deg_1_embedding() {
	vector<vector<int>> embedding_storage;
	vector<int> rot0;
	rot0.push_back(1);
	embedding_storage.push_back(rot0);

	vector<int> rot1;
	rot1.push_back(2); rot1.push_back(3); rot1.push_back(0);
	embedding_storage.push_back(rot1);

	vector<int> rot2;
	rot2.push_back(3); rot2.push_back(1);
	embedding_storage.push_back(rot2);

	vector<int> rot3;
	rot3.push_back(1); rot3.push_back(2);
	embedding_storage.push_back(rot3);

	return embedding_storage;
}

vector<vector<int>> create_sample_grid_nxn_embedding(int n) {
	vector<vector<int>> embedding_storage;
	for (int i = 0; i < n; i++) {
		/*if (i % 10 == 0) {
		printf("Read %d\n", i*n);
		}*/
		for (int j = 0; j < n; j++) {
			vector<int> rot;
			rot.reserve(4);
			if (i >= 1) rot.push_back((i - 1)*n + j);
			if (j + 1 < n) rot.push_back(i*n + j + 1);
			if (i + 1 < n) rot.push_back((i + 1)*n + j);
			if (j >= 1) rot.push_back(i*n + j - 1);
			embedding_storage.push_back(rot);
		}
	}
	return embedding_storage;
}

vector<vector<int>> create_sample_star_1xn_embedding(int n) {
	vector<vector<int>> embedding_storage;
	for (int i = 0; i < n - 1; i++) {
		vector<int> rot;
		rot.push_back(n - 1);
		embedding_storage.push_back(rot);
	}
	vector<int> rot_n_1;
	for (int i = 0; i < n - 1; i++) {
		rot_n_1.push_back(i);
	}
	embedding_storage.push_back(rot_n_1);
	return embedding_storage;

}

vector<vector<int>> create_planar_path(int n) {
	vector<vector<int>> embedding_storage;
	vector<int> rot0;
	rot0.push_back(1);
	embedding_storage.push_back(rot0);
	for (int i = 1; i < n - 1; i++) {
		vector<int> rot;
		rot.push_back(i - 1);
		rot.push_back(i + 1);
		embedding_storage.push_back(rot);
	}
	vector<int> rot_n_1;
	rot_n_1.push_back(n - 2);
	embedding_storage.push_back(rot_n_1);
	return embedding_storage;
}

void benchmarking() {
	time_t begin, end;
	cout<<"creating the embedding"<<endl;
	time(&begin);
	vector<vector<int>> embedding = create_sample_grid_nxn_embedding(1000);
	time(&end);
	double difference = difftime(end, begin);
	cout << "time taken for creating 1 mil vertices" << difference << " seconds." << endl;
	cout << "reading the embedding" << endl;
	time(&begin);
	planargraph g(1000000, embedding);
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken for reading the planar embedding of 1 mil vertices " << difference << " seconds." << endl;
	cout << "reading complete" << endl;
	sample_face_visitor face_visitor;
	time(&begin);
	planar_face_traversal(&g, face_visitor);
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken for visiting all faces " << difference << " seconds." << endl;

	time(&begin);
	int arc_index;
	for (int i = 0; i < g.m; i++) {
		arc_index = g.arc_map.find(g.arc_to_int64(g.arcs[i].source, g.arcs[i].sink))->second;
	}
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken to read arc_map" << difference << " seconds." << endl;


	time(&begin);
	planar_triangulate(&g);
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken to triangulate 1 mil g " << difference << " seconds." << endl;

	time(&begin);
	sample_bfs_visitor bfs_vis;
	bfs(&g.vertices[0], g, bfs_vis);
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken to bfs 1 mil g " << difference << " seconds." << endl;

	time(&begin);
	sample_dfs_visitor dfs_vis;
	dfs(&g.vertices[0], g, dfs_vis);
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken to dfs 1 mil g " << difference << " seconds." << endl;

	/*time(&begin);
	bfs_tree primal_bfs_tree(g, &g.vertices[0]);
	bfs(&g.vertices[0], g, primal_bfs_tree);
	time(&end);
	difference = difftime(end, begin);
	cout<<"time taken to build a primal bfs tree of mil g" << difference << " seconds." << endl;

	time(&begin);
	dual_tree dual_bfs_tree(primal_bfs_tree);
	dual_tree_builder tree_buider(dual_bfs_tree);
	planar_face_traversal(g, tree_buider);
	time(&end);
	difference = difftime(end, begin);
	cout<<"time taken to build a dual bfs tree of mil g " << difference << " seconds." << endl;*/

	time(&begin);
	std::vector<int> separator_container;
	find_low_radius_separator(&g, &g.vertices[0], separator_container);
	time(&end);
	difference = difftime(end, begin);
	cout << "time taken to find a separator of mil g " << difference << " seconds." << endl;

}

void r_division_of_grid(planargraph &g, int n, int r) {
	cout << "********************************************" << endl;
		cout << "testing division quality" << r << "-division of " << n << "x" << n << " grid" << endl;
	cout << "********************************************" << endl;
	//	r_division_by_lowradius_separator(g, r);
	r_division(g, r);
}
void r_division_quality_test() {
	vector<vector<int>> embedding = create_sample_grid_nxn_embedding(100);
	planargraph g(100 * 100, embedding);
	r_division_of_grid(g, 100, 10);
	r_division_of_grid(g, 100, 20);
	r_division_of_grid(g, 100, 30);
	r_division_of_grid(g, 100, 50);
	r_division_of_grid(g, 100, 70);
	r_division_of_grid(g, 100, 100);
}
