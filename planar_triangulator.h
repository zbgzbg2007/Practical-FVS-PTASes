/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
#include "planar_face_visitor.h"

struct  triangulator : face_traversal_visitor
{

	planargraph *g;
	int num_arcs_store_in_g = 0;
	arc *prev_arc0 = nullptr;
	arc *prev_arc1 = nullptr;


	triangulator(planargraph *arg_g) : g(arg_g) {
		num_arcs_store_in_g = g->m;
	};

	void begin_traversal() {}
	void begin_face() {}
	void next_vertex(vertex* v) {}

	void next_arc(arc* uv) {
		vertex *u = uv->source;
		vertex *v = uv->sink;
		if (prev_arc0 != nullptr && prev_arc1 != nullptr) {
			// recall prev0 = prev of prev1
			// prev1 = prev of uv
			if (*(uv->sink) != *(prev_arc0->source)) {
				// the current face is not triangular
				vertex *p = prev_arc0->sink;
				vertex *q = prev_arc0->source;	
				// add u->q  and q-> u arcs
				g->arcs[num_arcs_store_in_g] = g->create_arc(u->index, q->index);
				g->arcs[num_arcs_store_in_g + 1] = g->create_arc(q->index, u->index);
				arc *uq = &g->arcs[num_arcs_store_in_g];
				arc *qu = &g->arcs[num_arcs_store_in_g + 1];
				uq->index = num_arcs_store_in_g;
				uq->name = uq->index;
				uq->version = g->current_version + 1;	// update the arc version
				qu->index = num_arcs_store_in_g + 1;
				qu->name = qu->index;
				uq->rev = qu;
				qu->rev = uq;
				qu->version = g->current_version + 1;	// update the arc version
				// update next and prev of prev 0
				arc *next_of_prev0 = prev_arc0->nextarc;
				prev_arc0->nextarc = qu;
				qu->nextarc = next_of_prev0;
				next_of_prev0->prevarc = qu;
				qu->prevarc = prev_arc0;
				// update next and prev of uv
				uv->nextarc = uq;
				uq->prevarc = uv;
				uq->nextarc = prev_arc1->rev;
				prev_arc1->rev->prevarc = uq;
				prev_arc1 = qu;
				// since we are going to flipp arcs at the end, we will update
				// the neighbors of u and q after that
				num_arcs_store_in_g += 2;
			}
		}
		prev_arc0 = prev_arc1;
		prev_arc1 = uv;
	
	}

	void end_face() {
		prev_arc0 = nullptr;
		prev_arc1 = nullptr;
	}
	void end_traversal() {
		//printf("Triangulation finished\n");
		//printf("# of arcs: %d\n", num_arcs_store_in_g);
		// every face is guranteed to have face length of 3
		// but there could be parallel edges 
		// need to "flip" parallel added edges
		arc *uv, *vu, *uy, *yv, *vx, *xu;
		vertex *x, *y;
		// recall for each new arc uv and vu are located consecutively on the array
		for (int i = g->m; i < num_arcs_store_in_g ; i++) {
			if (g->arc_map.find(g->arc_to_int64(g->arcs[i].source, g->arcs[i].sink))  != g->arc_map.end()) {
				uv = &g->arcs[i];
				vu = uv->rev;
				vx = uv->rev->prevarc;
				xu = vx->rev->prevarc;
				uy = vu->rev->prevarc;
				yv = uy->rev->prevarc;
				x = vx->sink;
				y = uy->sink;
				// remove uv and vu
				xu->rev->prevarc = uy;	// ux->prevarc = uy
				uy->nextarc = xu->rev; // uy->nextarc = ux
				vx->nextarc = yv->rev;	// vx->nextarc = vy
				yv->rev->prevarc = vx;	// vy->prevarc = vx;
				// change uv to xy
				uv->source = x;
				uv->sink = y;
				vu->source = y;
				vu->sink = x;
				// insert xy and yx to the rotational system of x and y
				// now uv is xy and vu is yx
				yv->nextarc = vu;	
				vu->prevarc = yv;
				vu->nextarc = uy->rev;
				uy->rev->prevarc = vu;

				xu->nextarc = uv;
				uv->prevarc = xu;
				uv->nextarc = vx->rev;
				vx->rev->prevarc = uv;
			}
			g->arc_map.insert(arc_map_type::value_type(g->arc_to_int64(g->arcs[i].source, g->arcs[i].sink), i));
			g->arcs[i].source->arclist.push_back(&g->arcs[i]);
		}
		// update the neighbor list of new added arcs;
		
		g->m = num_arcs_store_in_g;
		g->num_version++;
		g->current_version++;		// update the graph version			
	}

};

void planar_triangulate(planargraph *g){
	triangulator trg(g);
	planar_face_traversal(g, trg);
}
