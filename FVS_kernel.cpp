/********************************************************************
Copyright 2018 Baigong Zheng

********************************************************************/

#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<string>
#include<algorithm>
#include<unordered_map>
#include<unordered_set>
#include<map>
#include<set>
#include<list>
#include<queue>
#include<stack>
#include<utility>
#include<iterator>
#include<tuple>
#include<random>
#include<ctime>
#include"FVS_kernel.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "graph.h"
#include "r_divisor.h"



namespace fvs_kernel {

class Union_Find_Set{
  private:
    std::unordered_map<int, int> set_id;
  public:
    void clear() {
      set_id.clear();
    }
    bool append(int e, int s) { 
      // add a new element e into set with set_id s
      if (set_id.find(e) != set_id.end())
        return false;
      set_id[e] = s;
      return true;
    }

    bool combine(int a, int b) {
      // union two sets containing a and b
     
      int x = find(a), y = find(b);
      if (x == y) return true;
      if (x == -1 || y == -1)
        return false;
      set_id[x] = y;
      return true;
    }
   
    int find(int e) {
      // return set id for given element e
      // if there is no such element, return -1
      if (set_id.find(e) == set_id.end())
        return -1;
      if (set_id[e] == e) 
        return e;
      int root = find(set_id[e]);
      set_id[e] = root;
      return root; 
    }
  
};




int FVS_kernel::get_degree(int v) {
  // return the degree of vertex v including parallel edges
  std::list<edge> edges = ids[v]->edges;
  int deg = 0;
  for (auto& e: edges) 
    deg += e.multi;
  return deg;
}

std::vector<int> FVS_kernel::get_neighbors(int v) {
  // return the list of vertex ids that are neighbors of v, not including v
  std::list<edge> edges = ids[v]->edges;
  std::vector<int> nv; 
  for (auto& e: edges) {
    if (e.endpts.first != v)
      nv.push_back(e.endpts.first);
    if (e.endpts.second != v)
      nv.push_back(e.endpts.second);
  }
  return nv;
}

bool FVS_kernel::simple(int a, int b) {
  // if there is an edge between a and b, and it is simple,
  // then return true
  auto p = std::make_pair(std::min(a, b), std::max(a, b));
  if (D1.find(p) == D1.end())
    return false;
  if (D1[p].first->multi == 1) 
    return true;
  return false; 
}

bool FVS_kernel::adjacent(int a, int b) {
  // return if a and b are adjacent
  auto p = std::make_pair(std::min(a, b), std::max(a, b));
  return D1.find(p) != D1.end();
}

void FVS_kernel::add_D2(int x, int y, int z) {
  // assume z is common neighbor of x and y
  // if z is in S(x, y), add z into D2[x, y]
  // update q3 if possible
  if (ids[z]->edges.size() > 3 || x == y)
    // z has too many edges
    return;

  std::vector<int> nv = get_neighbors(z);
  // find the next neighbor of z
  int u = -1;
  for (auto& v: nv)
    if (v != x && v != y)
      u = v;
  if (u != -1) {
  // check the edge zu is simple or not
    for (auto& e: ids[z]->edges)
      if (e.endpts.first == u || e.endpts.second == u)
        if (e.multi != 1)
          return;
  }
  
  // add z into D2[x, y]
  if (x > y) 
    std::swap(x, y);
  std::pair<int, int> p = std::make_pair(x, y);
  if (D2.find(p) == D2.end())
    D2[p] = std::unordered_set<int>();
  if (D2[p].find(z) == D2[p].end() && D2[p].size() == 2)
    q3.insert(p);

  D2[p].insert(z);
}

void FVS_kernel::remove_D2(int x, int y, int z) {
  // remove z from D2[x, y] if possible
  // update q3 if possible
  if (x > y)
    std::swap(x, y);
  std::pair<int, int> p = std::make_pair(x, y);
  if (D2.find(p) == D2.end())
    return;
  if (D2[p].find(z) != D2[p].end() && D2[p].size() == 3)
    q3.erase(p);
  D2[p].erase(z);
  if (D2[p].size() == 0)
    D2.erase(p); 
}


void FVS_kernel::remove_vertex(int v) {
  // remove only the vertex v, and update all data structures 
  // if there is isolated vertices after removing it, this function also maintain
  // all isolated vertices in G.
  // the optimal solution will not be modified here
  std::list<vertex>::iterator it = ids[v];
  std::list<edge>::iterator itt = (*it).edges.begin(); 
  
  if (D2.size() > 0) {
    // if D2 is not empty, remove v from D2

    std::vector<int> nv = get_neighbors(v);
    // remove v in values
    if (nv.size() == 3 || nv.size() == 2) {
      remove_D2(nv[0], nv[1], v);
      if (nv.size() == 3) {
        remove_D2(nv[1], nv[2], v);
        remove_D2(nv[0], nv[2], v); 
      } 
    }

    // remove v in keys
    for (auto& u: nv) {
      if (ids[u]->edges.size() == 3 || ids[u]->edges.size() == 2) {
        std::vector<int> nu = get_neighbors(u);
        for (auto& x: nu) 
          if (x != v) {
            if (x > v) {
              D2.erase(std::make_pair(v, x));
              q3.erase(std::make_pair(v, x));
            }
            else {
              D2.erase(std::make_pair(x, v));
              q3.erase(std::make_pair(x, v));
            }
          }
      }
    }
  }
  // remove edges incident to v
  while (itt != it->edges.end()) {
    std::pair<int, int> e = itt->endpts;
    int a = e.first, b = e.second;
    if (b == v)
      std::swap(a, b); // a == v
    // remove edge
    std::list<vertex>::iterator va = ids[a], vb = ids[b];
    std::list<edge>::iterator ea, eb;
    if (a == e.first)
      ea = D1[e].first, eb = D1[e].second;
    else
      eb = D1[e].first, ea = D1[e].second; 
    D1.erase(e);
    itt = it->edges.erase(ea);
    if (a != b) {
      vb->edges.erase(eb);
      if (d_flag) {
        int d_b = get_degree(b);
        d_map.erase(it_map[b]);
        it_map[b] = d_map.insert(std::make_pair(d_b, b)); 
      }
    }
  }
  G.erase(it);
  qs.erase(v);
  ids.erase(v);
  if (d_flag) {
    // update degree related structures
    d_map.erase(it_map[v]);
    it_map.erase(v);
  }
}

                        
void FVS_kernel::add_edge(int a, int b) {
  // add an edge between a and b 
  // if there exists such edge, increase its multiplicity
  // the multiplicity of an edge is at most 2
  // update G, D1, D2 and q3

  
  if (a > b)
    std::swap(a, b);
  std::pair<int, int> p = std::make_pair(a, b);
  int count = 0;
  int d_a = get_degree(a), d_b = get_degree(b);
  if (D1.find(p) != D1.end()) {
    // if there exists such edge in G
    count = D1[p].first->multi;
    if (count == 2)
      // there are two parallel edges, don't add it again
      return;
    // add parallel edges
    D1[p].first->multi ++;
    D1[p].second->multi ++;
  } 
  else {  
    // add the edge into G
    ids[a]->edges.push_back(edge(a, b, count+1));
    if (a != b)
      ids[b]->edges.push_back(edge(a, b, count+1));
    std::list<edge>::iterator ia = ids[a]->edges.end(), ib = ids[b]->edges.end();
    ia--, ib--;
    D1[p] = std::make_pair(ia, ib);
  }

  if (d_flag) {
    // update degree related structures
    d_map.erase(it_map[a]);   
    d_map.erase(it_map[b]);   
    it_map[a] = d_map.insert(std::make_pair(d_a+1, a)); 
    it_map[b] = d_map.insert(std::make_pair(d_b+1, b)); 
  }
   

  if (count == 1) {
    // there is one edge ab before adding, update D2
    if (ids[a]->edges.size() == 3) {
      // if a has three incident edges, update D2 for a
      std::vector<int> nv = get_neighbors(a);
      if (nv[0] == b)
        std::swap(nv[0], nv[2]);
      if (nv[1] == b)
        std::swap(nv[1], nv[2]);
      remove_D2(nv[0], nv[1], a);
    }
    if (ids[b]->edges.size() == 3) {
      // if b has three incident edges, update D2 for b
      std::vector<int> nv = get_neighbors(b);
      if (nv[0] == a)
        std::swap(nv[0], nv[2]);
      if (nv[1] == a)
        std::swap(nv[1], nv[2]);
      remove_D2(nv[0], nv[1], b);
    }
    return; 
  }

  // update D2 for a
  if (ids[a]->edges.size() <= 4) {
    std::vector<int> nv = get_neighbors(a); 
    if (nv.size() == 2 || nv.size() == 3) {
      // if need to add a into D2
      add_D2(nv[0], nv[1], a);
      if (nv.size() == 3) {
        add_D2(nv[0], nv[2], a);
        add_D2(nv[1], nv[2], a); 
      }
    }
    if (nv.size() == 4) {
      // if need to remove a from D2
      remove_D2(nv[0], nv[1], a); 
      remove_D2(nv[0], nv[2], a); 
      remove_D2(nv[0], nv[3], a); 
      remove_D2(nv[1], nv[2], a); 
      remove_D2(nv[1], nv[3], a); 
      remove_D2(nv[2], nv[3], a); 
    }
  }

  // update D2 for b
  if (ids[b]->edges.size() <= 4) {
    std::vector<int> nv = get_neighbors(b); 
    if (nv.size() == 2 || nv.size() == 3) {
      // if need to add b into D2
      add_D2(nv[0], nv[1], b);
      if (nv.size() == 3) {
        add_D2(nv[0], nv[2], b);
        add_D2(nv[1], nv[2], b); 
      }
    }
    if (nv.size() == 4) {
      // if need to remove b from D2
      remove_D2(nv[0], nv[1], b); 
      remove_D2(nv[0], nv[2], b); 
      remove_D2(nv[0], nv[3], b); 
      remove_D2(nv[1], nv[2], b); 
      remove_D2(nv[1], nv[3], b); 
      remove_D2(nv[2], nv[3], b); 
    }
  }
}

void FVS_kernel::contract_edge(int a, int b) {
  // if there is such an edge between a and b, 
  // contract the edge to b and remove a
  // if there are parallel edges between a and b, remove both vertices
  // update all data structures
  std::pair<int, int> p = std::make_pair(std::min(a, b), std::max(a, b));
  if (D1.find(p) == D1.end())
    return;
  
  if (D1[p].first->multi == 2) {
    // there are parallel edges between a and b
    remove_vertex(a);
    remove_vertex(b);
    opt.insert(b);
  }
  else {
    std::vector<int> nv = get_neighbors(a);
    remove_vertex(a);
    for (auto& x: nv) 
      if (x != b)
        add_edge(x, b); 
  } 
}

std::unordered_set<int> FVS_kernel::check15(int u) {
  // check vertex u for Rule 1 to 5 
  // also check other affected vertices for Rule 1 to 5
  // return a set of vertices that needs to be checked for Rule 6 to 11, which have 
  // at most four neighbors
  std::unordered_set<int> cands; // vertices to be checked
  std::unordered_set<int> res; //vertices to be returned
  cands.insert(u);
  while (!cands.empty()) { 
    int v = *(cands.begin());
    cands.erase(cands.begin());
    if (adjacent(v, v)) {
      // check self-loop
      std::vector<int> nv = get_neighbors(v);
      for (auto& i: nv)
        cands.insert(i);
      opt.insert(v);
      remove_vertex(v); 
      res.erase(v);
      continue;
    }
  
    if (ids[v]->edges.size() > 4)
      // these rules do not apply to high degree vertex 
      continue;
    int deg = get_degree(v);
    if (deg == 0) {
      // check isolated vertex
      remove_vertex(v);
      res.erase(v);
      continue;
    }

    std::vector<int> nv = get_neighbors(v);
    if (deg == 1) {
      // check degree-1 vertex
      cands.insert(nv[0]);
      remove_vertex(v); 
      res.erase(v);
      continue;
    }
    if (deg == 2) {
      // check degree-2 vertex
      if (ids[v]->edges.size() == 1) {
        // parallel edges
        remove_vertex(v);
        res.erase(v);
        v = nv[0];
        opt.insert(v);
        nv = get_neighbors(v);
        remove_vertex(v);
        res.erase(v);
        cands.erase(v);
        for (auto& i: nv)
          cands.insert(i);
      }
      else {
        contract_edge(v, nv[0]);
        cands.insert(nv[0]);
        cands.insert(nv[1]); 
        res.erase(v);
      }
      continue;
    } 
    if (deg == 3 && ids[v]->edges.size() == 2) {
      // degree-3 vertex with 2 neighbors
      int x;
      for (auto& e: ids[v]->edges) 
        if (e.multi > 1)
          x = (e.endpts.first == v ? e.endpts.second: e.endpts.first);
      remove_vertex(v);
      res.erase(v);
      for (auto& i: nv)
        cands.insert(i);
      opt.insert(x);
      nv = get_neighbors(x);
      remove_vertex(x);
      res.erase(x);
      cands.erase(x);
      for (auto& i: nv)
        cands.insert(i); 
      continue;
    }
    res.insert(v);
  } 
  return res;
} 
std::unordered_set<int> FVS_kernel::R6(int w, int v) {
  // apply Rule 6 for vertices w and v if possible
  // return affected vertices that are not processed
  if (w > v) 
    std::swap(w, v);
  std::pair<int, int> p = std::make_pair(w, v);
  std::unordered_set<int> res;
  if (D2.find(p) == D2.end() || D2[p].size() < 3)
    return res;
  opt.insert(w);
  opt.insert(v);
  std::vector<int> x; // store vertices to be removed
  x.push_back(v);
  x.push_back(w);
  std::unordered_set<int>::iterator it;
  int i;
  for (i = 0, it = D2[p].begin(); i < 3; i++, it++) 
    x.push_back(*it);
  std::vector<int> nv;
  for (i = 0; i < 5; i++) {
    nv = get_neighbors(x[i]);
    for (auto& j: nv)
      res.insert(j);
    remove_vertex(x[i]);
    res.erase(x[i]);
  }
  return res; 
}

void FVS_kernel::add_S_to_q(int a, int b) {
  // When an edge ab is added, the vertices in D2[a, b] needs to be added
  // into qs. This function adds D2[a, b] into qs if D2 contains (a, b) or
  // (b, a)
  int x = std::min(a, b), y = std::max(a, b);
  std::pair<int, int> p = std::make_pair(x, y);
  if (D2.find(p) != D2.end())
    for (auto& i: D2[p]) 
      qs.insert(i); 
}

std::unordered_set<int> FVS_kernel::R7(int u) {
  // apply Rule 7 for vertex u if possible
  // return affected vertices that are not processed
  std::unordered_set<int> res;
  if (ids[u]->edges.size() != 3) {
    return res; 
  }
  std::vector<int> nu; // neighbors of vertex u
  std::vector<edge> eu; // incident edges of vertex u
  for (auto& e: ids[u]->edges) {
    eu.push_back(e); 
    if (e.endpts.first == u)
      nu.push_back(e.endpts.second);
    else
      nu.push_back(e.endpts.first); 
  }
  if (eu[0].multi == 1 && eu[1].multi == 1) {
    // edges eu[0] and eu[1] are simple
    if (adjacent(nu[0], nu[2]) && adjacent(nu[1], nu[2]) ) { 
      // two pairs p1 and p2 are adjacent
      if (eu[2].multi == 2) {
         // there are parallel edges, then two vertices will be removed
        std::vector<int> nv = get_neighbors(nu[2]);
        for (auto& i: nv)
          res.insert(i);
        res.erase(u); 
      }
      else {
        res.insert(nu[0]);
        res.insert(nu[1]);
        res.insert(nu[2]); 
      }
      contract_edge(u, nu[2]);
      bool flag = false;
      if (adjacent(nu[0], nu[1]) == false)
        flag = true;
      add_edge(nu[0], nu[1]);
      if (flag)
        add_S_to_q(nu[0], nu[1]);
      return res;
    }
  }
  if (eu[0].multi == 1 && eu[2].multi == 1) {
    // edges eu[0] and eu[2] are simple
    if (adjacent(nu[0], nu[1]) && adjacent(nu[1], nu[2])) {
      if (eu[1].multi == 2) {
        std::vector<int> nv = get_neighbors(nu[1]);
        for (auto& i: nv)
          res.insert(i);
        res.erase(u); 
      }
      else {
        res.insert(nu[0]);
        res.insert(nu[1]);
        res.insert(nu[2]); 
      }
      contract_edge(u, nu[1]);
      bool flag = false;
      if (adjacent(nu[0], nu[2]) == false)
        flag = true;
      add_edge(nu[0], nu[2]);
      if (flag)
        add_S_to_q(nu[0], nu[2]);
      return res;
    }
  }
  if (eu[1].multi == 1 && eu[2].multi == 1) {
    // edges eu[1] and eu[2] are simple
    if (adjacent(nu[0], nu[1]) && adjacent(nu[0], nu[2])) {
      if (eu[0].multi == 2) {
        std::vector<int> nv = get_neighbors(nu[0]);
        for (auto& i: nv)
          res.insert(i);
        res.erase(u); 
      }
      else {
        res.insert(nu[0]);
        res.insert(nu[1]);
        res.insert(nu[2]); 
      }
      contract_edge(u, nu[0]);
      bool flag = false;
      std::pair<int, int> p = std::make_pair(std::min(nu[1], nu[2]), std::max(nu[1], nu[2]));
      if (adjacent(nu[1], nu[2]) == false) 
        flag = true;
      add_edge(nu[1], nu[2]);
      if (flag)
        add_S_to_q(nu[1], nu[2]);
      return res;
    }
  }
  return res;
}

bool FVS_kernel::equals(std::vector<int>nv, int a, int b, int c) {
  // check if nv is the set of a, b, and c
  if (nv.size() != 3)
    return false;
  for (auto& i: nv) 
    if (i != a && i != b && i != c)
      return false;
  return true; 
} 
std::unordered_set<int> FVS_kernel::applyR8 (int v1, int v2, int v3, int u1, int u2, int w1, int w2) {
  // given a set of vertices, apply Rule 8 for these vertices
  // return affected vertices that are not processed
  // assume w1 < w2
  std::unordered_set<int> res;
  // make sure w1 is a neighbor of v2
  std::vector<int> nv = get_neighbors(v2);
  for (auto& i: nv)
    if (i == w2)
      std::swap(w1, w2);

  contract_edge(v2, v1);
  bool flag = false;
  std::pair<int, int> p = std::make_pair(std::min(v3, w1), std::max(v3, w1));
  if (adjacent(v3, w1) == false) 
    flag = true;
  add_edge(v3, w1);
  if (flag)
    add_S_to_q(v3, w1);
  res.insert(v1);
  res.insert(w1);
  res.insert(v3);
  return res;

}


int FVS_kernel::the_other(int x, std::pair<int, int>& p) {
  // return the element in D2[p] that is different from x
  // assume p is in D2
  if (*D2[p].begin() == x)
    return *(++(D2[p].begin()));
  return *(D2[p].begin());

}

// Just check one vertex in config for R8, 9, 10
std::unordered_set<int> FVS_kernel::R8(int x) {
  // apply Rule 8 for vertex x as v2 if possible
  // return affected vertices that are not processed
  std::unordered_set<int> res; 
  if (ids[x]->edges.size() != 3 || get_degree(x) != 3)
    return res;

  std::vector<int> nv = get_neighbors(x); 
  std::unordered_set<int> temp;
  temp.insert(x);
  // x = v2 
  for (auto& v1: nv) {
    if (ids[v1]->edges.size() == 3) {
      temp.insert(v1);
      std::vector<int> w;
      std::vector<int> nv1 = get_neighbors(v1);
      for (auto& i: nv1)
        if (temp.find(i) == temp.end())
          w.push_back(i);
      if (w.size() == 2 && w[0] != w[1]) {
        // w0 and w1 do not appear in temp
        if (equals(nv1, w[0], w[1], x) == true) {
          // check neighbors of v1
          if (w[0] > w[1])
            std::swap(w[0], w[1]);
          std::pair<int, int> p = std::make_pair(w[0], w[1]);
          if (D2.find(p) != D2.end() && D2[p].size() > 1) {
            temp.insert(w[0]); 
            temp.insert(w[1]); 
            int u1 = the_other(v1, p);
            
            if (temp.find(u1) == temp.end() && ids[u1]->edges.size() == 3 && get_degree(u1) == 3) {
              temp.insert(u1);
              int u2;
              std::vector<int> nu1 = get_neighbors(u1);
              for (auto& i: nu1)
                if (i != w[0] && i != w[1]) {
                  u2 = i;
                  break;
                }
              if (temp.find(u2) == temp.end() && equals(nu1, w[0], w[1], u2) == true) {
                // check neighbors of u1
                temp.insert(u2);
                for (auto& v3: nv) 
                  if (temp.find(v3) == temp.end() && (equals(nv, v1, v3, w[0]) || equals(nv, v1, v3, w[1]))) 
                    // check neighbors of x
                    return applyR8(v1, x, v3, u1, u2, w[0], w[1]);
                  
                temp.erase(u2);
              }
              temp.erase(u1);
            } 
            temp.erase(w[0]);
            temp.erase(w[1]);
          } 
        }
      } 
      temp.erase(v1);
    } 
  }
  return res;
}
std::unordered_set<int> FVS_kernel::applyR9 (int u1, int u2, int u3, int u4, int w1, int w2) {
  // given a set of vertices, apply Rule 9 for these vertices
  // return affected vertices that are not processed
  // new vertex y will be u1
  std::unordered_set<int> res;
  // store the configuration
  int deg_u1 = get_degree(u1);
  L9.push_back(config9(u1, u2, u3, u4, w1, w2, u1, deg_u1));
  
  remove_vertex(u2);
  bool flag1 = false, flag2 = false;
  std::pair<int, int> p1 = std::make_pair(std::min(u1, u3), std::max(u1, u3));
  std::pair<int, int> p2 = std::make_pair(std::min(w1, u3), std::max(w1, u3));
  if (adjacent(u1, u3) == false) 
    flag1 = true;
  if (adjacent(w1, u3) == false) 
    flag2 = true;
  add_edge(u1, u3);
  add_edge(u3, w1);
  add_edge(u1, w1);
  add_edge(u1, w2);
  if (flag1)
    add_S_to_q(u1, u3);
  if (flag2)
    add_S_to_q(u3, w1);
  res.insert(u1);
  res.insert(u3);
  res.insert(w1);
  res.insert(w2);
  return res; 
}

std::unordered_set<int> FVS_kernel::R9(int x) {
  // apply Rule 8 for vertex x as u1 if possible
  // return affected vertices that are not processed
  // new vertex y will be u1
  std::unordered_set<int> res;
  if (ids[x]->edges.size() != 3) return res;
  std::unordered_set<int> temp;
  std::vector<int> nx = get_neighbors(x);
  temp.insert(x);
  int dx = get_degree(x);
  // x = u1
  for (auto& u2: nx) {
    if (ids[u2]->edges.size() == 3 && std::min(dx, get_degree(u2)) == 3) {
      temp.insert(u2);
      std::vector<int> w;
      for (auto& i: nx)
        if (i != u2) {
          w.push_back(i);
          temp.insert(i);
        }
      std::vector<int> nu2 = get_neighbors(u2);
      for (auto& u3: nu2) {
        if (temp.find(u3) == temp.end() && ids[u3]->edges.size() == 3 && get_degree(u3) == 3) {
          if (adjacent(x, u3) == false) { 
            // no edge between x and u3
            temp.insert(u3);
            std::vector<int> nu3 = get_neighbors(u3);
            for (auto& u4: nu3) 
              if (temp.find(u4) == temp.end()) {
                if (adjacent(x, u4) == false && adjacent(u2, u4) == false) {
                  // no edge between x and u4, no edge between u2 and u4
                  if (adjacent(u2, w[0]) && adjacent(u3, w[1]))
                    // edge between u2 and w0, also between u3 and w1
                    return applyR9(x, u2, u3, u4, w[0], w[1]);
                  if (adjacent(u2, w[1]) && adjacent(u3, w[0]))
                    // edge between u2 and w1, also between u3 and w0
                    return applyR9(x, u2, u3, u4, w[1], w[0]);
                }
              }
            temp.erase(u3);
          }
        }
      } 
      temp.erase(w[0]);
      temp.erase(w[1]);
      temp.erase(u2);
    } 
  }
  return res;
}

bool FVS_kernel::at_most_one_edge(int x, std::set<int> s) {
  // check if there is at most one edge incident to vertex x
  // and a vertex outside of s
  // asume |N(x)| <= 4
  int count = 0;
  s.insert(x);
  for (auto& e: ids[x]->edges) {
    if (s.find(e.endpts.first) == s.end() || s.find(e.endpts.second) == s.end())
      count++;
      if (e.multi > 1)
        return false;
  }
  if (count > 1) return false;
  return true;
}

std::unordered_set<int> FVS_kernel::applyR101(int v1, int v2, int u, int w1, int w2) {
  // check Lemma 7 for the given vertices
  // return affected vertices after the rule 10
  std::unordered_set<int> res;

  // make sure v2 and w1 are adjacent
  if (adjacent(v2, w1) == false) 
    std::swap(w1, w2);

  opt.insert(w1);
  std::vector<int> nw1 = get_neighbors(w1);
  for (auto& i: nw1)
    res.insert(i);
  remove_vertex(w1);
  return res;
}

std::unordered_set<int> FVS_kernel::R101(int x) {
  // assume vertex x has at most 4 neighbors
  // check Rule 10 and Lemma 7 for vertex x as v2
  // if Rule 10 applies, return affected vertices
  std::unordered_set<int> res;
  if (ids[x]->edges.size() > 4) return res;
  int nsize = ids[x]->edges.size();
  std::vector<int> nx = get_neighbors(x);

  // x = v2
  for (auto& v1: nx) {
    auto p12 = std::make_pair(std::min(x, v1), std::max(x, v1));
    if (ids[v1]->edges.size() == 3 && simple(x, v1)) {
      std::vector<int> nv1 = get_neighbors(v1);
      int w1, w2;
      if (x == nv1[0]) 
        w1 = std::min(nv1[1], nv1[2]), w2 = std::max(nv1[1], nv1[1]);
      else if (x == nv1[1]) 
        w1 = std::min(nv1[0], nv1[2]), w2 = std::max(nv1[0], nv1[1]);
      else if (x == nv1[2]) 
        w1 = std::min(nv1[1], nv1[0]), w2 = std::max(nv1[1], nv1[0]);
      
      std::pair<int, int> p = std::make_pair(w1, w2);
      if (D2.find(p) != D2.end() && D2[p].size() > 1) { 
        int u = the_other(v1, p);
        if (u != w1 && u != w2 && u != x && ids[u]->edges.size() == 3) { 
          if (adjacent(u, w1) && adjacent(u, w2)) { 
            std::set<int> s;
            s.insert(w1);
            s.insert(w2);
            if (at_most_one_edge(u, s)) {
              s.insert(v1);
              if (at_most_one_edge(x, s)) 
                return applyR101(v1, x, u, w1, w2);
            }
          }
        }
      }
    }
  }
  return res;
}

std::unordered_set<int> FVS_kernel::checkR102(int u1, int u2, int u3, int w1, int w2) {
  // check Rule 10 and Lemma 8 for the given vertices
  // before checking Rule 10 and Lemma 8, first check Rule 7 for u2
  // if Rule 10 applies, return affected vertices

  std::unordered_set<int> res;

  if (qs.find(u2) != qs.end()) {
    res = R7(u2);
    if (!res.empty()) return res;
  }

  // check neighbors of u2 
  std::vector<int> nu2 = get_neighbors(u2);
  int c = 0;
  for (auto& i: nu2) {
    if (i != w1 && i != w2 && i != u1 && i != u3)
      return res;
    if (i == u1 || i == u3)
      c++;
  }
  if (c != 2)
    return res;

  // check additional edge of u3
  std::set<int> s;
  s.insert(w1);
  s.insert(w2);
  s.insert(u2); 
  if (at_most_one_edge(u3, s) == false) return res;

  // check u1u2 and u2u3 are simple
  if (simple(u1, u2) == false || simple (u2, u3) == false) 
    return res;
  
  opt.insert(w1);
  std::vector<int> nw1 = get_neighbors(w1);
  for (auto& i: nw1) 
    res.insert(i);
  remove_vertex(w1);
  return res;
}

std::unordered_set<int> FVS_kernel::R102(int x) {
  // assume x has at most 4 neighbors
  // check Rule 10 and Lemma 8 for the vertex x as u1
  // if Rule 10 applies, return affected vertices
  std::unordered_set<int> res;
  if (ids[x]->edges.size() != 3) return res;
  std::vector<int> nx = get_neighbors(x);

  // x = u1
  if (nx.size() == 3) {
    for (auto& u2: nx) {
      if (ids[u2]->edges.size() <= 4) {
        std::vector<int> w;
        for (auto& i: nx) 
          if (i != u2)
            w.push_back(i);
        std::vector<int> nu2 = get_neighbors(u2);
        for (auto& u3: nu2) {
          if (u3 != x && u3 != w[0] && u3 != w[1] && ids[u3]->edges.size() <= 4) {
            res = checkR102(x, u2, u3, w[0], w[1]);
            if (!res.empty()) return res;
          }
        }
      } 
    } 
  } 

  return res; 
}

std::unordered_set<int> FVS_kernel::applyR11(int u, int v, int v1, int v2, int v3, int v4, 
                            int v5, int v6, int w1, int w2) {
  // check Rule 11 for the given vertices
  // before checking Rule 11, first check Rule 7 for v2, v3, ..., v6
  // assume v1 has been checked for Rule 7 
  // if possible, first apply Rule 7
  // return affected vertices
  std::unordered_set<int> res;

  // check R7 for vertices
  std::set<int> s;
  s.insert(v2);
  s.insert(v3);
  s.insert(v4);
  s.insert(v5);
  s.insert(v6);
  for (auto& i: s) {
    if (qs.find(i) != qs.end()) {
      res = R7(i);
      if (!res.empty()) return res;
    }
  }

  opt.insert(w1);
  std::vector<int> nw1 = get_neighbors(w1);
  remove_vertex(w1);
  for (auto& i: nw1)
    res.insert(i);
  return res;
}

std::unordered_set<int> FVS_kernel::R11(int x) {
  // assume x has at most 4 neighbors
  // try x as v1 in Rule 11
  // if the rule applies, return affected vertices
  std::unordered_set<int> res;
  if (ids[x]->edges.size() > 4) return res;
  std::unordered_set<int> temp;
  temp.insert(x);
  std::unordered_map<int, int> w;

  std::vector<int> nx = get_neighbors(x);
  for (auto& v2: nx) {
    if (ids[v2]->edges.size() <= 4 && simple(v2, x)) {
      temp.insert(v2);
      for (auto& u: nx) {
        if (u != v2 && adjacent(u, v2) == false && simple(u, x)) {
          temp.insert(u);
          for (auto& i: nx)
            if (temp.find(i) == temp.end()) {
              w[i] = 1;
            }
          std::vector<int> nv2 = get_neighbors(v2);
          for (auto& v3: nv2) {
            if (temp.find(v3) == temp.end() && ids[v3]->edges.size() <= 4 && simple(v2, v3)) {
            if (adjacent(u, v3) == false && adjacent(x, v3) == false) {
                temp.insert(v3);
                for (auto& i: nv2) 
                  if (temp.find(i) == temp.end()) {
                    if (w.find(i) != w.end()) w[i]++;
                    else w[i] = 1; 
                  }
                if (w.size() < 3) {
                  std::vector<int> nv3 = get_neighbors(v3);
                  for (auto& v4: nv3) {
                    if (temp.find(v4) == temp.end() && ids[v4]->edges.size() <= 4 && simple(v3, v4)) {
                      if (adjacent(u, v4) == false && adjacent(x, v4) == false && adjacent(v2, v4) == false) {
                        temp.insert(v4);
                        for (auto& i: nv3) 
                          if (temp.find(i) == temp.end()) {
                            if (w.find(i) != w.end()) w[i]++;
                            else w[i] = 1; 
                          }
                        if (w.size() < 3) {
                          std::vector<int> nv4 = get_neighbors(v4);
                          for (auto& v5: nv4) {
                            if (temp.find(v5) == temp.end() && ids[v5]->edges.size() <= 4 && simple(v4, v5)) {
                              if (adjacent(u, v5) == false && adjacent(x, v5) == false && adjacent(v2, v5) == false && adjacent(v3, v5) == false) {
                                temp.insert(v5);
                                for (auto& i: nv4) 
                                  if (temp.find(i) == temp.end()) {
                                    if (w.find(i) != w.end()) w[i]++;
                                    else w[i] = 1; 
                                  }
                                if (w.size() < 3) {
                                  std::vector<int> nv5 = get_neighbors(v5);
                                  for (auto& v6: nv5) {
                                    if (temp.find(v6) == temp.end() && ids[v6]->edges.size() <= 4 && simple(v5, v6)) {
                                      if (adjacent(u, v6) == false && adjacent(x, v6) == false && adjacent(v2, v6) == false && adjacent(v3, v6) == false  && adjacent(v4, v6) == false) {
                                        temp.insert(v6); 
                                        for (auto& i: nv5) 
                                          if (temp.find(i) == temp.end()) {
                                            if (w.find(i) != w.end()) w[i]++;
                                            else w[i] = 1; 
                                          }
                                        if (w.size() < 3) { 
                                          std::vector<int> nv6 = get_neighbors(v6);
                                          for (auto& v: nv6) {
                                            if (temp.find(v) == temp.end() && simple(v6, v) && adjacent(u, v) == false && adjacent(x, v) == false && adjacent(v2, v) == false && adjacent(v3, v) == false && adjacent(v4, v) == false && adjacent(v5, v) == false) {
                                              for (auto& i: nv6) 
                                                if (temp.find(i) == temp.end() && i != v) {
                                                  if (w.find(i) != w.end()) w[i]++;
                                                  else w[i] = 1; 
                                              }
                                              if (w.size() == 2) {
                                                std::vector<int> w1;
                                                for (auto& i: w)
                                                  w1.push_back(i.first);
                                                if (w[w1[0]] < w[w1[1]]) std::swap(w1[0], w1[1]);
                                                return applyR11(u, v, x, v2, v3, v4, v5, v6, w1[0], w1[1]);
                                              }
                                              for (auto& i: nv6) 
                                                if (temp.find(i) == temp.end() && i != v) {
                                                  w[i]--;
                                                  if (w[i] == 0)
                                                    w.erase(i);
                                                } 
                                            }
                                          } 
                                        } 
                                        for (auto& i: nv5) 
                                          if (temp.find(i) == temp.end()) {
                                            w[i]--;
                                            if (w[i] == 0)
                                              w.erase(i);
                                          }
                                        temp.erase(v6);
                                      }
                                    }
                                  }
                                }
                                for (auto& i: nv4) 
                                  if (temp.find(i) == temp.end()) {
                                    w[i]--;
                                    if (w[i] == 0)
                                      w.erase(i);
                                  }
                                temp.erase(v5);
                              }
                            } 
                          }
                        }
                        for (auto& i: nv3) 
                          if (temp.find(i) == temp.end()) {
                            w[i]--;
                            if (w[i] == 0)
                              w.erase(i);
                          }
                        temp.erase(v4);
                      }
                    }
                  }
                }
                for (auto& i: nv2) 
                  if (temp.find(i) == temp.end()) {
                    w[i]--;
                    if (w[i] == 0)
                      w.erase(i);
                  }
                temp.erase(v3);
              }
            }
          }
          w.clear();
          temp.erase(u);
        }
      }
      temp.erase(v2);
    }
  }
  return res;
}



std::list<vertex> FVS_kernel::compute_kernel() {
/* compute a linear kernel for graph G
   - Input: None
   - Output: a linear kernel of graph G 
*/


  std::vector<int> vs;
  for (auto& v: G)
    vs.push_back(v.id);
 
  // initialize D1
  for (auto& v: G) {
    for (std::list<edge>::iterator it = v.edges.begin(); it != v.edges.end(); it++) {
      if (it->multi > 2)
        it->multi = 2;
      std::pair<int, int> p = it->endpts;
      if (D1.find(p) == D1.end()) 
        D1[p] = std::make_pair(it, it);
      else {
        std::pair<std::list<edge>::iterator, std::list<edge>::iterator> t = D1[p];
        if (p.first == v.id)
          D1[p] = std::make_pair(it, t.first);
        else
          D1[p] = std::make_pair(t.first, it);
      }        
    }
  }

  // remove vertices with self-loop and isolated vertices
  for(auto& v: vs) { 
    if (ids.find(v) == ids.end())
      continue;
    if (adjacent(v, v)) { 
      remove_vertex(v);
      opt.insert(v);
    }
    else if (ids[v]->edges.size() == 0)
      remove_vertex(v);
  }

  // initialize D2
  for (auto& v: G) { 
    if (v.edges.size() == 2) {
      std::vector<int> nv = get_neighbors(v.id);
      std::pair<int, int> p;
      int a = nv[1], b = nv[0];
      if (a > b)
        std::swap(a, b);
      p = std::make_pair(a, b);
      if (D2.find(p) == D2.end())
        D2[p] = std::unordered_set<int>();
      D2[p].insert(v.id); 
    }
    if (v.edges.size() == 3) {
      std::vector<int> nv; // neighbor vertices 
      std::vector<edge> ne; // neighbor edges
      for (auto& e: v.edges) {
        int a = e.endpts.first, b = e.endpts.second;
        if (a != v.id) 
          nv.push_back(a);
        else 
          nv.push_back(b);
        ne.push_back(e);
      }
      // update possible pairs by adding vertex v
      std::pair<int, int> p;
      if (ne[0].multi == 1) {
        // necessary to check parallel edges
        int a = nv[1], b = nv[2];
        if (a > b)
          std::swap(a, b);
        p = std::make_pair(a, b);
        if (D2.find(p) == D2.end())
          D2[p] = std::unordered_set<int>();
        D2[p].insert(v.id);
      }
      if (ne[1].multi == 1) {
        // necessary to check parallel edges
        int a = nv[0], b = nv[2];
        if (a > b)
          std::swap(a, b);
        p = std::make_pair(a, b);
        if (D2.find(p) == D2.end())
          D2[p] = std::unordered_set<int>();
        D2[p].insert(v.id); 
      }
      if (ne[2].multi == 1) {
        // necessary to check parallel edges
        int a = nv[1], b = nv[0];
        if (a > b)
          std::swap(a, b);
        p = std::make_pair(a, b);
        if (D2.find(p) == D2.end())
          D2[p] = std::unordered_set<int>();
        D2[p].insert(v.id); 
      } 
    }
  } 

  
  // initialize q3
  for (auto& kv: D2) 
    if (kv.second.size() > 2)
      q3.insert(kv.first);
  
  // apply rules 1~5 
  // this may change the graph G, so we iterate through a distinct vector instead of G
  for (auto& v: vs)
    if (ids.find(v) != ids.end())
      check15(v);

  // initialize qs
  for (auto& v: G) {
    if (v.edges.size() <= 4) 
      qs.insert(v.id);
  }


  while (!q3.empty() || !qs.empty()) {
    std::unordered_set<int> res;
    if (!q3.empty()) {
      std::pair<int, int> p = *(q3.begin());
      q3.erase(q3.begin());
      res = R6(p.first, p.second);
    }
    else {
      std::list<int> to_delete;
      for (auto& v: qs) {
        res = R7(v);
        if (!res.empty()) 
          break;
        res = R8(v);
        if (!res.empty())
          break;
        res = R9(v);
        if (!res.empty())
          break;
        res = R101(v);
        if (!res.empty())
          break;
        res = R102(v);
        if (!res.empty())
          break;
        res = R11(v);
        if (!res.empty())
          break;
        to_delete.push_back(v);
      }
      for (auto& v: to_delete) 
        qs.erase(v); 
    } 
    if (!res.empty()) {
      for (auto& x: res) {
        if (ids.find(x) == ids.end())
          continue;
        std::unordered_set<int> q = check15(x);
        for (auto& y: q) {
          qs.insert(y);
        }
      }
    } 
  }

  return G;
}



std::list<vertex> construct(std::string f) {
  // construct a simple graph from a file containing adjacency lists for all vertices
  // each line in the file has the same form: vertex_id neighbor1 neighbor2 neighbor3 ...
  // resulting edges satisfy endpt1 <= endpt2

  std::ifstream graph;
  graph.open(f);
  std::list<vertex> H;
  std::string temp;

  while (std::getline(graph, temp)) {
    // read numbers from a line
    std::istringstream buffer(temp);
    std::vector<int> line((std::istream_iterator<int>(buffer)), std::istream_iterator<int>());
    
    // construct vertex structure
    int a = line[0], n = line.size();
    std::list<edge> l;
    vertex v(a, l);
    for (int i = 1; i < n; i++) {
      int b = line[i];
      edge e(a, b, 1);
      v.edges.push_back(e);
    }

    H.push_back(v);
  }


  graph.close(); 
  return H;
}




std::list<vertex> construct(std::list<std::vector<int> >& edges) { 
  // construct a graph according to a list of edges: endpt1, endpt2, multi
  // resulting edges satisfy endpt1 <= endpt2
 
  std::list<vertex> H;
  for (auto& e: edges) {
    if (e[0] > e[1])
      std::swap(e[0], e[1]);
  }
  std::map<int, std::list<vertex>::iterator> maps;
  for (auto& e: edges) {
    if (maps.find(e[0]) == maps.end()) {
      std::list<edge> l;
      H.push_back(vertex(e[0], l));
      maps[e[0]] = --H.end();
    }
    if (maps.find(e[1]) == maps.end()) {
      std::list<edge> l;
      H.push_back(vertex(e[1], l));
      maps[e[1]] = --H.end();
    }
    maps[e[0]]->edges.push_back(edge(e[0], e[1], e[2]));
    if (e[0] != e[1])
      maps[e[1]]->edges.push_back(edge(e[0], e[1], e[2])); 
  }
  return H;
}

FVS_kernel::FVS_kernel(std::list<vertex>& X) {
  // initialize original graph as the deep copy of given graph X
  for (auto& v: X) {
    G.push_back(vertex(v));
    ids[v.id] = --G.end(); 
  } 
}

std::vector<std::vector<int> > compute_embed(std::list<std::vector<int> >& input) {
  // compute embedding adjacency list for given graph 
  // each vector in input only contains neighbors
  // each vector in result contains vertex id and its neighbors
    graphP theGraph = gp_New();

    my_gp_Read(theGraph, input, input.size()); 
    gp_Embed(theGraph, EMBEDFLAGS_PLANAR);

    gp_SortVertices(theGraph);
 
    std::vector<std::vector<int> > embedding;
    
    my_gp_Write(theGraph, embedding); 

    gp_Free(&theGraph);
  
    return embedding;
}

void FVS_kernel::rDivision(rDiv& div, int r) {
  // compute r-division for kernel 
  // the result will be in div
   
  if (G.size() == 0) 
    return;

  
  // map new vertex id (0 to n-1) to original vertex id
  std::map<int, int> embed_map1;
  // map original vertex id to new vertex id (0 to n-1)
  std::map<int, int> embed_map2;

  int i = 0;
  for (auto& v: G) {
    embed_map1[i] = v.id;
    embed_map2[v.id] = i;
    i += 1;
  }
  
  // construct adjacency list for kernel G
  std::list<std::vector<int> > input;
  for (auto& v: G) {
    std::vector<int> v_list;
    for (auto& e: v.edges) {
      int u = e.endpts.first != v.id ? e.endpts.first : e.endpts.second;
      v_list.push_back(embed_map2[u]); 
    }
    input.push_back(v_list);
  } 
  std::vector<std::vector<int> > embed = compute_embed(input);

  
  r_division(embed, r, div);

  // recover vertex id
  int n = div.boundary.size();
  for (int j = 0; j < n; j++) {
    div.boundary[j] = embed_map1[div.boundary[j]];
  }
  // recover vertex id and parallel edges
  for (auto& g: div.graphs) {
    int n = g.size();
    for (auto& v: g) {
      v.id = embed_map1[v.id];
      for (auto& e: v.edges) {
        int a = e.endpts.first, b = e.endpts.second;
        a = embed_map1[a];
        b = embed_map1[b];
        if (a > b)
          std::swap(a, b);
        e.endpts = std::make_pair(a, b);
        e.multi = D1[e.endpts].first->multi; 
      }
    }
  }
  add_boundary(div.boundary);
}





void FVS_kernel::recover_solution(bool minimal/*=true*/) { 
  // recover original solution from kernel solution
  // assume opt contains kernel solution
  // modify opt to a solution for original graph
  // remove redundant vertices of opt
  // if minimal is false, we do not remove redundant vertices



  if (L9_count != 0) {
    // process Rule 9 configurations from approximate function
    if (L9_count > 0)
      L9_count = L9.size() - L9_count;
    else // L9_count = -1 and L9 is empty before applying approximation function, then we need to process all configurations
      L9_count = L9.size();
    for (int i = 0; i < L9_count; i++) {
      config9 x = L9.back();
      std::vector<int> common;
      if (opt.find(x.y) != opt.end())
        common.push_back(x.y);
      if (opt.find(x.w1) != opt.end())
        common.push_back(x.w1);
      if (opt.find(x.w2) != opt.end())
        common.push_back(x.w2);
      if (opt.find(x.u3) != opt.end())
        common.push_back(x.u3);
      if (common.size() > 0) {
        if (common.size() > 1) {
          for (auto& i: common)
            opt.erase(i);
          opt.insert(x.w1);
          opt.insert(x.w2);
        }
        else {
          opt.erase(x.y);
          if (x.deg_u1 == 3)
            opt.insert(x.u2);
          else
            opt.insert(x.u1);  
        }
      }
      L9.pop_back();
    }
  }

  if (minimal) {
    // try to remove redundant vertices from bound or greedy
    std::list<int> temp; // candidate vertices
    if (bound.empty() == false) {
      for (auto& v: bound)
        temp.push_back(v);
      bound.clear();
    }
    else { 
      if (greedy.empty() == false) {
        for (auto& v: greedy) {
          if (opt.find(v) != opt.end())
            temp.push_front(v);
        }
        greedy.clear(); 
      }
    }

    if (temp.empty() == false) {
      std::unordered_set<int> vs; // set of remaining vertices
      for (auto& v: G)
        if (opt.find(v.id) == opt.end())
          vs.insert(v.id); 
      
      // build an union-find set for vs
      Union_Find_Set ufs;
      for (auto& v: vs) {
        ufs.append(v, v);
      }


      for (auto& v: vs) {
        for (auto& e: ids[v]->edges) {
          int u = v == e.endpts.first ? e.endpts.second : e.endpts.first;
          if (vs.find(u) != vs.end())
            ufs.combine(v, u);
        }
      }
      // for each vertex in temp set, try to add back to graph
      std::unordered_set<int> redundancy;
      for (auto& v: temp) {
        std::unordered_set<int> nv;
        bool flag = false;
        for (auto& e: ids[v]->edges) {
          int u = v == e.endpts.first ? e.endpts.second : e.endpts.first;
          if (vs.find(u) != vs.end()) {
            if (e.multi > 1 || u == v) {
              // parallel edges or self-loop
              flag = true;
              break;
            }
            
            int root = ufs.find(u);
            if (nv.find(root) == nv.end()) {
              // u is not in the same component as previous neighbors of v
              nv.insert(root);
            }
            else {
              flag = true;
              break; 
            }
          }
        }
        if (flag == false) {
          // add redundant vertex to S and redundant set 
          redundancy.insert(v);
          vs.insert(v);
          ufs.append(v, v);
          for (auto& u: nv) {
            ufs.combine(v, u);
          }
        } 
      }
      
      // remove redundant vertices
      for (auto& v: redundancy) 
        opt.erase(v);
      
      temp.clear();
    } 
  }
  
  for (auto& v: temp_opt)
    opt.insert(v);

  while (L9.empty() == false) {
    config9 x = L9.back();
    std::vector<int> common;
    if (opt.find(x.y) != opt.end())
      common.push_back(x.y);
    if (opt.find(x.w1) != opt.end())
      common.push_back(x.w1);
    if (opt.find(x.w2) != opt.end())
      common.push_back(x.w2);
    if (opt.find(x.u3) != opt.end())
      common.push_back(x.u3);
    if (common.size() > 0) {
      if (common.size() > 1) {
        for (auto& i: common)
          opt.erase(i);
        opt.insert(x.w1);
        opt.insert(x.w2);
      }
      else {
        opt.erase(x.y);
        if (x.deg_u1 == 3)
          opt.insert(x.u2);
        else
          opt.insert(x.u1);  
      }
    }
    L9.pop_back();
  }

}

void FVS_kernel::add_solution(std::vector<int> newset) {
  // add vertices of newset into current solution set opt
  for (auto& x: newset) {
    opt.insert(x);
  } 
}


void FVS_kernel::add_solution(std::unordered_set<int> newset) {
  // add vertices of newset into current solution set opt
  for (auto& x: newset) {
    opt.insert(x);
  } 
}




void FVS_kernel::approximate() {
  // combine greedy choice with reduction rules to compute a FVS solution for kernel in the vector greedy
  // this function will affect all the internal structures
  
  L9_count = L9.size();
  if (L9_count == 0) L9_count--; // make sure the count is non-zero
  d_flag = true; // make sure all basic operations maintaining additional structures
  std::list<vertex> X;
  // initialize those structures
  for (auto& v: G) {
    X.push_back(vertex(v));
    int d = get_degree(v.id);
    it_map[v.id] = d_map.insert(std::make_pair(d, v.id)); 
  }
  
  for (auto& v: opt) {
    temp_opt.push_back(v);
  }
  opt.clear();

  int i = 0;
  while (G.empty() == false) {
    i++;
    int vi = d_map.rbegin()->second;
    greedy.push_back(vi);
    std::vector<int> nvi = get_neighbors(vi);
    remove_vertex(vi);
    opt.insert(vi);

    // clean G by reduction rules
    for (auto& u: nvi) {
      if (ids.find(u) == ids.end())
        continue;
      std::unordered_set<int> q = check15(u);
      for (auto& y: q) {
        qs.insert(y);
      } 
    }
    
    while (i%step_size == 0 && (!q3.empty() || !qs.empty())) { // can fix step size here
    
      std::unordered_set<int> res;
      if (!q3.empty()) {
        std::pair<int, int> p = *(q3.begin());
        q3.erase(q3.begin());
        res = R6(p.first, p.second);
      }
      else {
        std::list<int> to_delete;
        for (auto& v: qs) {
          res = R7(v);
          if (!res.empty()) 
            break;
          res = R8(v);
          if (!res.empty())
            break;
          res = R9(v);
          if (!res.empty())
            break;
          res = R101(v);
          if (!res.empty())
            break;
          res = R102(v);
          if (!res.empty())
            break;
          res = R11(v);
          if (!res.empty())
            break;
          to_delete.push_back(v);
        }
        for (auto& v: to_delete) 
          qs.erase(v); 
      } 
      if (!res.empty()) {
        for (auto& x: res) {
          if (ids.find(x) == ids.end())
            continue;
          std::unordered_set<int> q = check15(x);
          for (auto& y: q) {
            qs.insert(y);
          }
        }
      } 
    } 
  }
  // restore G and ids
  for (auto& v: X) {
    G.push_back(vertex(v));
    ids[v.id] = --G.end();
  } 
}




bool is_FVS(std::list<vertex>& H, indices& ids, std::unordered_set<int> s) {
  // return if s is FVS in graph H

  // get vertex set x = V-s
  std::vector<int> x;
  Union_Find_Set ufs;
  for (auto& v: H) {
    if (s.find(v.id) == s.end()) {
      x.push_back(v.id); 
      ufs.append(v.id, v.id);
    }
  }
  
  // test if x is a forest
  for (auto& v: x) {
    for (auto& e: ids[v]->edges) {
      int a = e.endpts.first, b = e.endpts.second;
      if (a == b) {
        // find self-loop for v
        return false;
      }
      if (a == v)
        continue;
      int u = a == v ? b : a; 
      if (s.find(u) == s.end()) {
        // u is not in s

        if (e.multi > 1)
          // find parallel edges
          return false;

        int z = ufs.find(u), y = ufs.find(v);
        if (z == y)
          return false;
        else {
          // u is not in the same components as v
          ufs.combine(z, y);
        }
      } 
    }
  }

  return true; 
}


std::vector<int> compute_FVS(std::list<vertex>& G) {
  // compute FVS for graph G by trivial enumeration
  std::vector<int> vs; // vertex set of G
  std::list< std::vector<int> > s; // candidate sets
  std::vector<int> res; // FVS result 
  indices ids;

  std::list<vertex>::iterator it;
  for (it = G.begin(); it != G.end(); it++)
    ids[it->id] = it;

  // test if G is a tree
  std::unordered_set<int> empty;
  if (is_FVS(G, ids, empty))
    return res;

  for (auto& v: G) {
    vs.push_back(v.id); 
    std::vector<int> w(1, v.id);
    s.push_back(w);
  }

  sort(vs.begin(), vs.end());
  int n = vs.size();
  std::unordered_map<int, int> id_map; // map vertex id to its index in vs
  for (int i = 0; i < n; i++)
    id_map[vs[i]] = i;

  
  for (int i = 1; i < n; i++) {
    // try all candidate subsets of size i
    for (auto& t: s) { 
      // transform set t to unordered_set
      std::unordered_set<int> temp;
      for (auto& x: t)
        temp.insert(x);

      if (is_FVS(G, ids, temp)) {
        // if result X is a forest 
        res = t;
        return t;
      }
    } 

    // generate all candidate subsets of size i+1 
    std::list<std::vector<int> > ns;
    for (auto& a: s) {
      int last = a.back();
      for (int j = id_map[last]+1; j < n; j++) {
        std::vector<int> w(a);
        w.push_back(vs[j]);
        ns.push_back(w); 
      }
    }
    s = ns;
  }

  return res;
}


std::list<std::vector<int> > generate_edges(std::string file) {
  // generate edge list from file
  std::ifstream graph;
  graph.open(file);
  int a, b, c;
  // construct edge list 
  std::list<std::vector<int> > edges;
  while (graph >> a >> b >> c) {
    std::vector<int> e;
    e.push_back(std::min(a, b));
    e.push_back(std::max(a, b));
    e.push_back(c);
    edges.push_back(e);
  }
  graph.close();
  return edges;
}


int Approx_FVS::get_degree(int v) {
  int d = 0;
  for (auto& e: ids[v]->edges) 
    d += e.multi;
  return d;
}

Approx_FVS::Approx_FVS(std::list<vertex>& H) {
  
  // deep copy of H
  for (auto& v: H) {
    G.push_back(vertex(v));
    ids[v.id] = --G.end();
  } 
 
  // initialize D1
  for (auto& v: G) {
    for (std::list<edge>::iterator it = v.edges.begin(); it != v.edges.end(); it++) {
      if (it->multi > 2)
        it->multi = 2;
      std::pair<int, int> p = it->endpts;
      if (D1.find(p) == D1.end()) 
        D1[p] = std::make_pair(it, it);
      else {
        std::pair<std::list<edge>::iterator, std::list<edge>::iterator> t = D1[p];
        if (p.first == v.id)
          D1[p] = std::make_pair(it, t.first);
        else
          D1[p] = std::make_pair(t.first, it);
      }        
    }
  }
}

std::vector<int> Approx_FVS::remove_vertex(int v, double c) {
  // remove vertex v
  // decrease weight of neighbors of v by c for each incident edge
  // return its neighbors
  std::vector<int> nv;
  if (ids.find(v) == ids.end())
    return nv;
  std::list<edge>::iterator it = ids[v]->edges.begin();
  while (it != ids[v]->edges.end()) {
    int a = it->endpts.first, b = it->endpts.second; 

    if (c != 0) {
      // update weight of endpoint x
      int x = a != v ? a : b; 
      int d = get_degree(x);
      double w = d == it->multi ? 0 : ((it_map[x]->first * d) - (c * it->multi)) / (d - it->multi);
      w_map.erase(it_map[x]);
      it_map[x] = w_map.insert(std::make_pair(w, x));
    }
    // remove the edge
    if (a != b) {
      if (a != v) {
        ids[a]->edges.erase(D1[it->endpts].first);
        nv.push_back(a);
      }
      else {
        ids[b]->edges.erase(D1[it->endpts].second);
        nv.push_back(b);
      }
    }
    D1.erase(it->endpts); 
    it = ids[v]->edges.erase(it);
  }
  G.erase(ids[v]);
  ids.erase(v);
  if (c != 0) {
    w_map.erase(it_map[v]);
    it_map.erase(v);
  }
  return nv;
}

void Approx_FVS::clean(bool removed, std::vector<int> s, double c) {
  // removed: if this function is called after the remove_vertex function
  // s: if after remove_vertex function, this is the set of neighbors of the removed vertex
  // c: decrease weight for each edge, used in remove_vertex function
  // remove vertices with degree less than 2
  // the resulting graph will not contain such vertices
  
  std::set<int> q;
  if (removed) {
    for (auto& v: s) {
      if (get_degree(v) < 2)
        q.insert(v);
    }
  }
  else {
    for (auto& v: G) {
      if (get_degree(v.id) < 2)
        q.insert(v.id);
    }
  }

  while (q.empty() == false) { 
    int v = *(q.begin());
    q.erase(q.begin());
    std::vector<int> nv = remove_vertex(v, c);
    for (auto& u: nv) {
      if (get_degree(u) < 2 && q.find(u) == q.end())
        q.insert(u);
    }
  } 
}



std::vector<int> Approx_FVS::approximate() {
  // compute 2-approximation for FVS in G

  // deep copy of G
  std::list<vertex> H;
  for (auto& v: G) 
    H.push_back(vertex(v));
  
  // remove self-loop vertices 
  for (auto& v: G) {
    for (auto& e: v.edges) {
      if (e.endpts.first == e.endpts.second) {
        solution.push_back(v.id);
        break;
      }
    }
  }
  
  for (auto& v: solution) {
    remove_vertex(v, 0);
  }
 
  std::vector<int> fvs; // temporary solution
  std::set<int> vs; // a vertex set 
  std::list<vertex> X; // copy of G
  indices x_ids; // ids for X

  // clean G
  clean(false, fvs, 0);

  
  // initialize weights
  for (auto& v: G) {
    it_map[v.id] = w_map.insert(std::make_pair(1.0/get_degree(v.id), v.id));
    vs.insert(v.id); 
    X.push_back(vertex(v));
    x_ids[v.id] = --X.end();
  }
  
  
  // while G is not empty
  while (G.empty() == false) {

    // add a new vertex to temp solution, remove it
    int vi = w_map.begin()->second;
    double c = w_map.begin()->first;
    
    fvs.push_back(vi);
    std::vector<int> nvi = remove_vertex(vi, c);
    
    // clean G
    clean(true, nvi, c); 
  }
  

  // reverse fvs
  std::reverse(fvs.begin(), fvs.end());

  // build a vertex set vs = V / FVS
  for (auto& v: fvs) {
    vs.erase(v);
  }
    
  // build an union-find set for vs
  Union_Find_Set ufs;
  for (auto& v: vs) {
    ufs.append(v, v);
  }
  for (auto& v: vs) {
    for (auto& e: x_ids[v]->edges) {
      int u = v == e.endpts.first ? e.endpts.second : e.endpts.first;
      if (vs.find(u) != vs.end())
        ufs.combine(v, u);
    }
  }

  // for each vertex in fvs, try to add back to graph
  std::unordered_set<int> redundancy;
  for (auto& v: fvs) {
    std::unordered_set<int> nv;
    bool flag = false;
    for (auto& e: x_ids[v]->edges) {
      int u = v == e.endpts.first ? e.endpts.second : e.endpts.first;
      if (vs.find(u) != vs.end()) {
        if (e.multi > 1) {
          // parallel edges
          flag = true;
          break;
        }
        
        int root = ufs.find(u);
        if (nv.find(root) == nv.end()) {
          // u is not in the same component as previous neighbors of v
          nv.insert(root);
        }
        else {
          flag = true;
          break; 
        }
      }
    }
    if (flag == false) {
      // add redundant vertex to S and redundant set 
      redundancy.insert(v);
      vs.insert(v);
      ufs.append(v, v);
      for (auto& u: nv) {
        ufs.combine(v, u);
      }
    } 
  }
  
  // remove redundant vertices
  for (auto& v: fvs) {
    if (redundancy.find(v) == redundancy.end())
      solution.push_back(v);
  }
 


  return solution;
}




pid_t child_pid = -1;

void kill_child(int sig) {
  kill(-child_pid, SIGKILL); 
}

void set_path(std::string path) { 
  // set FPT algorithm path
  const std::string fvs_path = path; 
}

void set_time_limit(int seconds) {
  // set time limit for FPT algorithm
  const int time_limit = seconds;
}


std::vector<int> compute_FVS_FPT(std::list<vertex>& G) {
  // compute FVS for graph G by calling FPT algorithm 
  // if it does not terminate then return an empty solution

  std::vector<int> fvs; //solution
  // write to file
  std::string input_file = temp_input;
  std::string output_file = temp_output;
  std::ofstream f;
  f.open(input_file);
  for (auto& v: G) {
    int a = v.id;
    for (auto& e: v.edges) { 
      int b, c;
      if (a == e.endpts.first)
        b = e.endpts.second;
      else
        b = e.endpts.first;
      c = e.multi;
      if (b <= a) {
        f<<a<<" "<<b<<std::endl;
        if (c > 1)
          f<<a<<" "<<b<<std::endl;
      }
    }
  }
  f.close();

  int status;
  signal(SIGALRM, &kill_child);
  child_pid = fork();
  
  // multiple processing
  if (child_pid > 0) {
    // parent process
    alarm(time_limit);
    wait(&status);
    if (status == 0) {
      // child process finished successfully 
      // read from result file
      std::ifstream ff;
      ff.open(output_file);
      int v;
      while (ff >> v) 
        fvs.push_back(v);
      ff.close();
    }
    else {
      // child process didn't finish in limited time and was killed
 
      // just return an empty solution so the final result will not be a FVS
      return fvs; 
    } 
  }
  else if (child_pid == 0) {
    // child process
    // run the FPT code
    setpgid(getpid(), getpid());
    std::string cmds = fvs_path+"/run.sh < " + input_file + " > " + output_file;
    const char * mycmd = cmds.c_str();
    system(mycmd);
    _exit(0);

  }
  return fvs;
}



std::vector<int> recurse_kernel_FVS(std::list<vertex>& G, int region_size/*=20*/, bool bound/*=false*/) {
  // compute FVS for graph G
  // apply kernel to each level of separator until each region is smaller than region_size
  // if the boolean value bound is true, then the size of the boundary is appended to the end of returning vector




  int total = 0; // record the total size of the vertices computed by exact algorithm for regions
  std::list<FVS_kernel> storage_K; // keep all intermediate kernels
  std::list<rDiv> storage_R; // keep all intermediate rDivs
  std::list<std::list<std::list<FVS_kernel>::iterator> > HK; // hierarchy for kernels iterators in r-division 
  std::list<std::list<std::list<rDiv>::iterator> > HR; // hierarchy for r-division

  storage_K.push_back(FVS_kernel(G));
  storage_R.push_back(rDiv());
  std::list<FVS_kernel>::iterator itK = storage_K.begin();
  std::list<rDiv>::iterator itR = storage_R.begin();
  itK->compute_kernel(); 
  int rr = region_size;
  if (itK->get_kernel_size() > rr) {
    int r = itK->get_kernel_size() / 2; // r-division
    itK->rDivision(*itR, r);
  }
  else {
    std::list<vertex> kernel_g = itK->get_kernel();
    std::vector<int> fvs = compute_FVS(kernel_g);
    itK->add_solution(fvs);
    itK->recover_solution();
    std::vector<int> res;
    std::unordered_set<int> opt = itK->get_opt();
    for (auto& v: opt)
      res.push_back(v);
    if (bound) 
      // no separator is applied, so no boundary is added
      res.push_back(0);
    return res;
  }

  std::list<std::list<FVS_kernel>::iterator> level_K;
  std::list<std::list<rDiv>::iterator> level_R;
  level_K.push_back(itK);
  level_R.push_back(itR);
  HK.push_back(level_K);
  HR.push_back(level_R);
  std::list<std::list<std::list<rDiv>::iterator> >::iterator litR = HR.begin(); // level iterator for r-division
 
  while (litR != HR.end()) {
    // recursively apply r-division and kernel
    std::list<std::list<FVS_kernel>::iterator> level_K;
    std::list<std::list<rDiv>::iterator> level_R;
    
    for (auto& R: *litR) {
      // iterate through a level
      for (auto& g: R->graphs) {
        // iterate through subgraphs in an r-division 
        storage_K.push_back(FVS_kernel(g));
        storage_R.push_back(rDiv());
        itK = storage_K.end();
        itR = storage_R.end();
        itK--, itR--;
        itK->compute_kernel(); 
        int size_k = itK->get_kernel_size();
        if (size_k > rr) {
          // graph is large, continue division
          int r = itK->get_kernel_size() / 2; // r-division
          itK->rDivision(*itR, r); 
        }
        else { 
          // graph is small, solve the problem
          if (size_k > 0) {
            std::vector<int> fvs;
            std::list<vertex> kernel_g = itK->get_kernel();
            if (size_k > rr) {
              fvs_kernel::Approx_FVS appx(kernel_g);
              fvs = appx.approximate(); 
            }
            else {
              fvs = compute_FVS_FPT(kernel_g);
            }
            itK->add_solution(fvs);
            total += fvs.size();
          }
        }
        level_K.push_back(itK);
        level_R.push_back(itR);
      } 
    }
    if (level_K.empty() == false) {
      HK.push_back(level_K);
      HR.push_back(level_R);
    }
    litR++;
  }
  litR--; // last level in hierarchy
  std::list<std::list<std::list<FVS_kernel>::iterator> >::iterator litK = HK.end(); // level iterator for kernels
  bool flag = false; // if solve the original graph 
  while (flag == false) {
    // compute solutions for each graph in the hierarchy

    if (litR == HR.begin()) 
      // this will be the first level
      flag = true;

    
    std::list<std::list<std::list<FVS_kernel>::iterator> >::iterator prev_litK = litK;
    prev_litK--; // the same level as litR
    std::list<std::list<FVS_kernel>::iterator>::iterator prev_ititK = prev_litK->begin(); // point to the first kernel in the level of litR
    std::list<std::list<FVS_kernel>::iterator>::iterator ititK; 
    if (litK != HK.end())
      ititK = litK->begin(); // point to kernel in the next level of prev_litK
    for (auto& R: *litR) {
      // iterate through a level
      for (auto& g: R->graphs) {
        // iterate through subgraphs in a division
        (*prev_ititK)->add_solution((*ititK)->get_opt()); 
        ititK++;
      }
      (*prev_ititK)->recover_solution();
      prev_ititK++;
    }
    litR--;
    litK--;
  }


  std::vector<int> res;
  std::unordered_set<int> opt = (*((HK.begin())->begin()))->get_opt();
  for (auto& v: opt)
    res.push_back(v);
  if (bound)
    // append the size of the boundary vertices
    res.push_back(opt.size()-total);
  return res;
}

std::vector<int> ptas_FVS(std::list<vertex>& X, int region_size/*=20*/, bool minimal/*=true*/) { 
  // compute FVS for graph X by original PTAS: find kernel, apply r-division on kernel and then apply exact algorithm on each region
  // if minimal is false, we do not remove redundant vertices in solution
  

  std::vector<int> res;
  // apply FVS kernel 
  FVS_kernel K(X); 
  K.compute_kernel();



  // compute r-division for kernel 
  fvs_kernel::rDiv div;
  int r = region_size; // r-division size
  K.rDivision(div, r);
  K.add_solution(div.boundary);


  // solve each region in r-division
  for (auto& g: div.graphs) {
    std::vector<int> fvs;
    FVS_kernel Kg(g);
    Kg.compute_kernel();
    std::list<vertex> kernel_g = Kg.get_kernel();
    if (kernel_g.empty() == false) {
      // apply exact algorithm
      fvs = compute_FVS_FPT(kernel_g);
      //if (fvs.empty()) return res; // can not finish FPT algorithm in given time
    }    


    Kg.add_solution(fvs);
    Kg.recover_solution();
  

    //fvs = fvs_kernel::compute_FVS(kernel_g);
    K.add_solution(Kg.get_opt());
  } 


  // recover depending on if remove redundant vertices
  K.recover_solution(minimal);
  std::unordered_set<int> opt = K.get_opt();
  for (auto& v: opt)
    res.push_back(v);


  return res; 
}


void remove_node(std::list<vertex>& G, int v, indices& ids) {
  // for local search function 
  std::list<edge>::iterator it = ids[v]->edges.begin();
  while (it != ids[v]->edges.end()) {
    int a = it->endpts.first, b = it->endpts.second;
    std::list<edge>::iterator eit;
    if (a != v) {
      eit = ids[a]->edges.begin();
      while (eit != ids[a]->edges.end()) {
        int c = eit->endpts.first, d = eit->endpts.second;
        if (c == v || d == v) {
          ids[a]->edges.erase(eit);
          break;
        }
        eit++;
      } 
    }
    if (b != v) {
      eit = ids[b]->edges.begin();
      while (eit != ids[b]->edges.end()) {
        int c = eit->endpts.first, d = eit->endpts.second;
        if (c == v || d == v) {
          ids[b]->edges.erase(eit);
          break;
        }
        eit++;
      } 
    }
    it++;
  }
  G.erase(ids[v]);
  ids.erase(v);
}
void add_node(std::list<vertex>& G, vertex& v, std::unordered_set<int>& fvs, indices& ids) {
  // for local search function
  // v is not in fvs
  vertex u(v);
  std::list<edge>::iterator it = u.edges.begin();
  while (it != u.edges.end()) {
    int a = it->endpts.first, b = it->endpts.second;
    if (fvs.find(a) != fvs.end() || fvs.find(b) != fvs.end())
      it = u.edges.erase(it);
    else {
      if (a != u.id) {
        ids[a]->edges.push_back(edge(*it));
      }
      if (b != u.id) {
        ids[b]->edges.push_back(edge(*it)); 
      }
      it++;
    }
  }
  G.push_back(u);
  ids[u.id] = --G.end();
}

std::vector<int> local_search(std::list<vertex>& X, std::vector<int>& fvs, int size, int iterations, std::vector<int>& improvement, unsigned seed/*=41*/) {
  // apply local search for given graph and solution 
  // size: the size of exchange set
  // iterations: the number of times to apply switch 
  // if iterations is negative, the function will stop when there is no imporvement for a number of consecutive rounds of search. This number is set as the absolute value of the iterations.
  // if iterations is zero, do nothing just return


  if (iterations == 0) return fvs;
  std::vector<int> res(fvs);
  std::list<vertex> G; // acyclic graph X - fvs
  std::unordered_set<int> temp_res;  
  indices idsG, idsX;
  std::map<int, int> degrees;
  for (auto& v: fvs)
    temp_res.insert(v);
  for (std::list<vertex>::iterator it = X.begin(); it != X.end(); it++) {
    int vid = it->id;
    idsX[vid] = it;
    degrees[vid] = 0;
    for (auto& e: it->edges)
      degrees[vid] += e.multi;
    // construct G 
    if (temp_res.find(vid) == temp_res.end()) {
      vertex u(*it);
      std::list<edge>::iterator eit = u.edges.begin();
      while (eit != u.edges.end()) {
        int a = eit->endpts.first, b = eit->endpts.second;
        if (temp_res.find(a) != temp_res.end() || temp_res.find(b) != temp_res.end())
          eit = u.edges.erase(eit);
        else
          eit++; 
      }
      G.push_back(u);
      idsG[vid] = --G.end();
    }
  }


  std::minstd_rand0 rand (seed);
  bool stop = false;
  int ii = 0;
  while (stop == false) {
    // generate a random set
    std::unordered_set<int> s1; 
    int total = temp_res.size();
    if (size > temp_res.size()) {
      size = temp_res.size();
      for (auto& x: res) {
        s1.insert(x);
        temp_res.erase(x);
        add_node(G, *idsX[x], temp_res, idsG);
      }
    }
    else {
      while (s1.size() < size) {
        int i = rand() % total;
        int x = res[i];
        //if (s1.find(x) == s1.end() && degrees[x] < 6) {
        if (s1.find(x) == s1.end()) {
          s1.insert(x);
          temp_res.erase(x);
          add_node(G, *idsX[x], temp_res, idsG);
        }
      }
    }



    // compute fvs for G
    FVS_kernel K(G); 
    K.compute_kernel();


    std::list<vertex> kernel_g = K.get_kernel();
    std::vector<int> temp_sol;
    if (kernel_g.empty() == false) {
      // try exact FPT algorithm
      temp_sol = compute_FVS_FPT(kernel_g);
      if (temp_sol.empty()) {
        // can not finish FPT algorithm in given time
        // try approximation algorithm
        K.set_step_size(20);
        K.approximate();
      }
      else 
        K.add_solution(temp_sol); 
    }


    K.recover_solution(); 

    std::unordered_set<int> temp_opt = K.get_opt();
    if (temp_opt.size() < s1.size()) {
      // find better solution
      for (auto& v: temp_opt) {
        temp_res.insert(v);
        remove_node(G, v, idsG);
      }
      improvement.push_back(s1.size()-temp_opt.size());
    }
    else {
      // find worse solution
      for (auto& v: s1) {
        temp_res.insert(v);
        remove_node(G, v, idsG);
      }
      improvement.push_back(0);
    }


    res.clear();
    for (auto& v: temp_res)
      res.push_back(v);

    // set terminal condition 
    if (iterations > 0) {
      ii++;
      if (ii >= iterations)
        stop = true;
    }
    else {
      if (improvement.back() == 0) 
        ii++;
      else ii = 0;
      if (ii >= -iterations)
        stop = true;
    }
  } 

  return res;
}





  // config variables
  const std::string fvs_path = "fvs-solver"; 
  const int time_limit = 15;

  const std::string temp_input = "temp_inputs"; //for FPT
  const std::string temp_output = "temp_output"; // for FPT 

}
