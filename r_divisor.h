/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planar_separator.h"
#include "planar_triangulator.h"
#include "FVS_kernel.h"

typedef std::vector<vertex*> vertex_container;
typedef std::vector<arc*> arc_container;

typedef fvs_kernel::vertex kvertex;
typedef fvs_kernel::edge kedge;
typedef std::list<kvertex> kgraph;

void create_subplanargraph(planargraph &g_subgraph, graph_components &g_components, int comp_id) {
	for (int i = 0; i < g_subgraph.n; i++) {
		g_subgraph.vertices[i].id =  g_components.vertices_of_components[comp_id][i]->id;	// recall id never change
		g_components.vertices_of_components[comp_id][i]->name = i; // update the name of the corresponding vertex to update arcs later on
		g_subgraph.vertices[i].index = i;
	}
	// update arcs of the planar subgraph
	vertex *source, *sink;
	for (int i = 0; i < g_subgraph.m; i++) {
		// change the name of the graph to point to the index of the corresponding arc in the subgraph
		// this will help in updating the rotational system of the subgraph
		g_components.arcs_of_components[comp_id][i]->name = i;
		source = &g_subgraph.vertices[g_components.arcs_of_components[comp_id][i]->source->name];
		sink = &g_subgraph.vertices[g_components.arcs_of_components[comp_id][i]->sink->name];
		g_subgraph.arcs[i].source = source;
		g_subgraph.arcs[i].sink = sink;
		g_subgraph.arcs[i].version = 0;
		g_subgraph.arcs[i].mark = false;
		g_subgraph.arcs[i].index = i;
		g_subgraph.arcs[i].name = i;
		source->arclist.push_back(&g_subgraph.arcs[i]);
	}
	//g_subgraph.print();
	// update the nextarc, prevarc and rev pointers
	arc *nextarc, *prevarc;
	int source_comp_id = -1;
	int sink_comp_id = -1;
	for (int i = 0; i < g_subgraph.m; i++) {
		g_subgraph.arcs[i].rev = &g_subgraph.arcs[g_components.arcs_of_components[comp_id][i]->rev->name];
		nextarc = g_components.arcs_of_components[comp_id][i];
		// recall that source and/or sink of nextarc could be in the separator
		do {
			nextarc = nextarc->nextarc;
			source_comp_id = g_components.vertex_to_comp_id[nextarc->source->index];
			sink_comp_id = g_components.vertex_to_comp_id[nextarc->sink->index];
		} while (source_comp_id < 0 || sink_comp_id < 0);
		g_subgraph.arcs[i].nextarc = &g_subgraph.arcs[nextarc->name];

		prevarc = g_components.arcs_of_components[comp_id][i];
		do {
			prevarc = prevarc->prevarc;
			source_comp_id = g_components.vertex_to_comp_id[prevarc->source->index];
			sink_comp_id = g_components.vertex_to_comp_id[prevarc->sink->index];
		} while (source_comp_id < 0 || sink_comp_id < 0);
		g_subgraph.arcs[i].prevarc = &g_subgraph.arcs[prevarc->name];
	}
	//g_subgraph.check_rotational_system();
	//printf("*****************vertex map*****************\n");
	//for (int i = 0; i < g_components.vertices_of_components[comp_id].size(); i++) {
	//	printf("%d = %d\n", g_components.vertices_of_components[comp_id][i]->id, g_components.vertices_of_components[comp_id][i]->name);
	//}
}

void r_division_by_lowradius_separator(planargraph &g, int r) {
	int sep_count = 0;
	planar_triangulate(&g);
	//int *separator_container = new int[g.n];
	std::vector<int> boundary_vertices;
	std::vector<int> separator_container;
	find_low_radius_separator(&g, &g.vertices[0], separator_container);
	boundary_vertices.insert(boundary_vertices.end(), separator_container.begin(), separator_container.end());
	//printf("%d-th separation\n", sep_count);
	sep_count++;
	g.reset();
	// find subgraphs of g after removing the separator
	graph_components g_components;
	g_components.init(&g);
	separtor_bfs(g, g_components, separator_container);
	std::list<planargraph> big_graph_lists;	// list of subgraphs that have more than r vertices
	std::list<planargraph> small_graph_lists;// list of subgraphs that have at most r vertices
	for (int comp_id = 0; comp_id < g_components.num_components; comp_id++) {
		if (g_components.vertices_of_components[comp_id].size() > r) {
			// the component is big
			big_graph_lists.push_back(planargraph());
			big_graph_lists.back().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
			create_subplanargraph(big_graph_lists.back(), g_components, comp_id);
			big_graph_lists.back().init_arc_map();
		}
		else {
			small_graph_lists.push_back(planargraph());
			small_graph_lists.back().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
			create_subplanargraph(small_graph_lists.back(), g_components, comp_id);
		}	
	}

	
	while (!big_graph_lists.empty()) {
		//printf("processsing a big graph\n");
		planar_triangulate(&big_graph_lists.back());
		//big_graph_lists.back().print();
		separator_container.clear();
		find_low_radius_separator(&big_graph_lists.back(), &big_graph_lists.back().vertices[0], separator_container);
		vertex* vertices = big_graph_lists.back().vertices;
		for (int i = 0; i < separator_container.size(); i++) {
			boundary_vertices.push_back(vertices[separator_container[i]].id);
		}
		//if(sep_count % 10 == 2)printf("%d-th separation\n", sep_count);
		sep_count++;
		big_graph_lists.back().reset();
		// clear g_components and relcaim the mamory
		graph_components g_components(&big_graph_lists.back());
		separtor_bfs(big_graph_lists.back(), g_components, separator_container);
		for (int comp_id = 0; comp_id < g_components.num_components; comp_id++) {
			if (g_components.vertices_of_components[comp_id].size() > r) {
				// the component is big
				big_graph_lists.push_front(planargraph());
				big_graph_lists.front().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
				create_subplanargraph(big_graph_lists.front(), g_components, comp_id);
				//big_graph_lists.front().print();
			}
			else {
				small_graph_lists.push_back(planargraph());
				small_graph_lists.back().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
				create_subplanargraph(small_graph_lists.back(), g_components, comp_id);
			}
		}
		big_graph_lists.pop_back();

	}
	//printf("boundary size of the r-division: %d\n", boundary_vertices.size());
	//printf("Separation at %d quality = %lf\n", r, 1.0 - (double)boundary_vertices.size()/g.n);
}

void r_division(planargraph &g, int r, std::vector<int> &boundary_vertices, std::list<planargraph> &small_graph_lists){
	//printf("Start %d-division\n",r);
	time_t begin, end;
	time(&begin);
	int sep_count = 0;

	std::vector<int> separator_container;

	/*find_separator(g,separator_container);
	boundary_vertices.insert(boundary_vertices.end(), separator_container.begin(), separator_container.end());
	sep_count++;*/

	// find subgraphs of g after removing the separator
	//std::cout << "Find connected components of the input graph" << std::endl;
	graph_components g_components;
	g_components.init(&g);
	separtor_bfs(g, g_components, separator_container);
	std::list<planargraph> big_graph_lists;	// list of subgraphs that have more than r vertices
	for (int comp_id = 0; comp_id < g_components.num_components; comp_id++) {
		if (g_components.vertices_of_components[comp_id].size() > r) {
			// the component is big
			big_graph_lists.push_back(planargraph());
			big_graph_lists.back().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
			create_subplanargraph(big_graph_lists.back(), g_components, comp_id);
			big_graph_lists.back().init_arc_map();
		}
		else {
			small_graph_lists.push_back(planargraph());
			small_graph_lists.back().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
			create_subplanargraph(small_graph_lists.back(), g_components, comp_id);
		}
	}


	while (!big_graph_lists.empty()) {
		separator_container.clear();
                big_graph_lists.back().init_arc_map();
		find_separator(big_graph_lists.back(), separator_container);
		vertex* vertices = big_graph_lists.back().vertices;
		for (int i = 0; i < separator_container.size(); i++) {
			boundary_vertices.push_back(vertices[separator_container[i]].id);
		}
		sep_count++;
		// clear g_components and relcaim the mamory
		graph_components g_components(&big_graph_lists.back());
		separtor_bfs(big_graph_lists.back(), g_components, separator_container);
		for (int comp_id = 0; comp_id < g_components.num_components; comp_id++) {
			if (g_components.vertices_of_components[comp_id].size() > r) {
				// the component is big
				big_graph_lists.push_front(planargraph());
				big_graph_lists.front().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
				create_subplanargraph(big_graph_lists.front(), g_components, comp_id);
			}
			else {
				small_graph_lists.push_back(planargraph());
				small_graph_lists.back().init(g_components.vertices_of_components[comp_id].size(), g_components.arcs_of_components[comp_id].size());
				create_subplanargraph(small_graph_lists.back(), g_components, comp_id);
			}
		}
		big_graph_lists.pop_back();
	}
	//printf("boundary size of the r-division: %d\n", boundary_vertices.size());
	//printf("Separation at %d quality = %lf\n", r, 1.0 - (double)boundary_vertices.size() / g.n);
	time(&end);
	double difference = difftime(end, begin);
	//printf("Total time for r-division is  %.2lf seconds.\n", difference);
}

void r_division(planargraph &g, int r) {
	std::vector<int> boundary_vertices;
	std::list<planargraph> small_graph_lists;
	r_division(g, r, boundary_vertices, small_graph_lists);
}

void print_k_graph(kgraph &kg) {
	std::cout << "*******************************************" << std::endl;
	std::cout << "KERNEL GRAPH" << std::endl;
	std::cout << "******************************************" << std::endl;
	for (auto &v : kg) {
		std::cout << v.id << ":";
		for (auto &e : v.edges) {
			std::cout << "\t" << (e.endpts.second == v.id ? e.endpts.first : e.endpts.second);
		}
		std::cout << std::endl;
	}
	std::cout << "******************************************" << std::endl;
}

void to_FSV_kernel_graph(kgraph  &fvs_kernel_graph, planargraph &g) {
	for (int i = 0; i < g.n; i++) {
		std::list<kedge> edge_list;
		for (std::vector<arc*>::iterator arc_it = g.vertices[i].arclist.begin(); arc_it != g.vertices[i].arclist.end(); ++arc_it) {
			edge_list.push_back(kedge(g.vertices[i].id, (*arc_it)->sink->id, 1));
		}
		fvs_kernel_graph.push_back(kvertex(g.vertices[i].id, edge_list));
	}
}

void to_kernel_rDiv(fvs_kernel::rDiv &div, std::vector<int> &boundary_vertices, std::list<planargraph> &graphs) {
	for (auto v : boundary_vertices) {
		div.boundary.push_back(v);
	}

	for (std::list<planargraph>::iterator it = graphs.begin(); it != graphs.end(); it++) {
		kgraph kg;
		to_FSV_kernel_graph(kg, (*it));
		div.graphs.push_back(kg);
	}
}


// the output is rDiv structure in FVS_kernel namespace
void r_division(planargraph &g, int r, fvs_kernel::rDiv &div) {
	std::vector<int> boundary_vertices;
	std::list<planargraph> small_graph_lists;
	r_division(g, r, boundary_vertices, small_graph_lists);
	to_kernel_rDiv(div, boundary_vertices, small_graph_lists);
}





void r_division(std::vector<std::vector<int>>& Boyer_embedding, int r, fvs_kernel::rDiv &div){

/*
  ///////// for debug
  std::string filename = "r_div_debug";
  std::ofstream f;
  f.open(filename);
  f << "start rDivision" << std::endl;
  /////////
*/ 
 
   // reverse neighbor order for each vertex
   for (auto& v: Boyer_embedding) {
      std::reverse(v.begin(), v.end()); 
    } 
   //f << "finish reverse" << std::endl;

/*
   // convert Boyer_embedding to vector embedding
   // my rewriting
   std::vector<std::vector<int>> embedding_storage;
   for (std::list<std::vector<int>>::iterator it = Boyer_embedding.begin(); it != Boyer_embedding.end(); it++) {
     std::vector<int>::iterator it1 = it->begin(), it2 = it->end();
     it1++;
     std::vector<int> v(it1, it2);
     embedding_storage.push_back(v);
   }
   //////////////
 


	// convert Boyer_embedding to vector embedding
	std::vector<std::vector<int>> embedding_storage;
	for(int i = 0; i < Boyer_embedding.size(); i++){
		embedding_storage.push_back(std::vector<int>());
	}
	for(std::list<std::vector<int>>::iterator it = Boyer_embedding.begin(); it != Boyer_embedding.end(); ++it){
		int i = (*it).front();
		for(int j = 1 ; j < (*it).size(); j++){
			embedding_storage[i].push_back((*it)[j]);
		}
	}
        f << "finish converting" << std::endl;
*/
	//planargraph g(embedding_storage.size(), embedding_storage);
	planargraph g(Boyer_embedding.size(), Boyer_embedding);
        //f << "finish planargraph" << std::endl;
	std::vector<int> boundary_vertices;
	std::list<planargraph> small_graph_lists;
	r_division(g, r, boundary_vertices, small_graph_lists);
        //f << "finish r_division" << std::endl;
	to_kernel_rDiv(div, boundary_vertices, small_graph_lists);	
        //f << "finish to_kernel" << std::endl;
        //f.close();
}



