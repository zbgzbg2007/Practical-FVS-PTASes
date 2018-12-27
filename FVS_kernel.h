/********************************************************************
Copyright 2018 Baigong Zheng

********************************************************************/

#ifndef FVS_KERNEL_H
#define FVS_KERNEL_H


namespace fvs_kernel {
  struct pair_compare { 
    bool operator()(const std::pair<int, int>& p1, const std::pair<int, int>& p2) const {
      if (p1.first == p2.first)
        return p1.second < p2.second;
      return p1.first < p2.first; 
    } 
  };
  
  
  struct pair_hash {
    size_t operator()(const std::pair<int, int>& p) const {
      std::hash<int> h;
      return h(p.first) ^ h(p.second);
    }
  };
  
  
  // define edge element in the adjacency list
  struct edge {
    std::pair<int, int> endpts; // assume p1 <= p2
    int multi; // multiplicity of the edge
    edge(int a, int b, int m=1) {
      int p1 = std::min(a, b);
      int p2 = std::max(a, b);
      endpts = std::make_pair(p1, p2);
      multi = m;
    } 
    edge(const edge& e) {
      endpts = std::make_pair(e.endpts.first, e.endpts.second);
      multi = e.multi;
    }
  };
  
  struct vertex {
    std::list<edge> edges;
    int id;
    vertex(int i, std::list<edge>& l) {
      id = i;
      for (auto& e: l) 
        edges.push_back(edge(e));
    }
    vertex(const vertex& v) {
      id = v.id;
      for (auto& e: v.edges)
        edges.push_back(edge(e));
    }
    vertex() {
      id = 0;
    }
  };
  
  
  struct rDiv {
    std::vector<int> boundary;
    std::list<std::list<vertex> > graphs;
  
  };
  
  
  
  // map from a pair of vertex id to a pair of iterators of edges
  //typedef unordered_map<pair<int, int>, pair<list<edge>::iterator, list<edge>::iterator>, pair_hash > adjacency;
  typedef std::map< std::pair<int, int>, std::pair<std::list<edge>::iterator, std::list<edge>::iterator>, pair_compare > adjacency;
  
  
  
  // map from a pair of vertex id to a set of ids
  //typedef unordered_map<pair<int, int>, unordered_set<int>, pair_hash > neighbor;
  typedef std::map< std::pair<int, int>, std::unordered_set<int>, pair_compare > neighbor;
  
  
  // map from vertex id to its iterator in graph
  typedef std::unordered_map<int, std::list<vertex>::iterator> indices;
  
  
  
  struct config9 {
    // configuration for Rule 9
    // used for recover origianl solution from kernel solution
    int u1, u2, u3, u4, w1, w2, y, deg_u1;
    config9(int a, int b, int c, int d, int e, int f, int g, int deg) {
      u1 = a;
      u2 = b;
      u3 = c;
      u4 = d;
      w1 = e;
      w2 = f;
      y = g; 
      deg_u1 = deg;
    } 
  };
  
  class Approx_FVS{ 
  
    private:
  
      indices ids; // map id to iterator in graph
      adjacency D1; // map edge to iterators in graph
      std::list<vertex> G; // original graph
      std::multimap<double, int> w_map; // map weight to id
      std::unordered_map<int, std::multimap<double, int>::iterator> it_map; // map id to iterator in w_map
      std::vector<int> solution; // final solution
      int get_degree(int v); 
      std::vector<int> remove_vertex(int v, double c);
      void clean(bool removed, std::vector<int> s, double c);
      
    public:  
      Approx_FVS(std::list<vertex>& H); // initialize input graph
      std::vector<int> approximate(); // compute 2-approximation
      std::vector<int> get_solution() { return solution; } 
  
  };
  
  
  class FVS_kernel{
  
    private:
      // the vertices belonging to the optimal solution
      std::unordered_set<int> opt;
  
      // map from vertex id to its iterator in graph K
      indices ids;
  
      // The value is a pair of two iterators for the edges in 
      // the adjacency lists of the corresponding endpoints. 
      // The first iterator corresponds the first endpoint.
      adjacency D1;
      
      // the value is a set of common neighbors of the edge's endpoints such that each vertex in the set has at least one more neighbor different from the two endpoints
      neighbor D2;
  
      // the set stores all pairs whose value in D2 has size greater than 2
      //unordered_set<pair<int, int>, pair_hash > q3;
      std::set<std::pair<int, int>, pair_compare > q3;
      
      // the set stores vertices with at most four neighbors
      std::unordered_set<int> qs;
      
      // FVS kernel graph
      std::list<vertex> G; 
      
  
      // The list stores the subgraph used in Rule 9
      // Given a solution for kernel, this will be used to 
      // find the solution of original graph 
      std::list<config9> L9;
  
      // boundary vertices from r-division
      std::unordered_set<int> bound;
  
      // store the vertices chosen greedily in order as temporary solution
      std::vector<int> greedy;
      // flag true means maintaining the following two structures in basic operation functions
      bool d_flag = false; 
      std::multimap<int, int> d_map; // map degree to id
      std::unordered_map<int, std::multimap<int, int>::iterator> it_map; // map id to iterator in d_map
      std::vector<int> temp_opt;
      int L9_count = 0;
      int step_size = 1; //frequency parameter: the number of greedy steps between two applications of kernelization alg
      bool lower_bound = false; // compute lower bound or approximation in the approximate function?
  
      int get_degree(int v);
      std::vector<int> get_neighbors(int v);
      bool simple(int a, int b);
      bool adjacent(int a, int b);
      void add_D2(int x, int y, int z);
      void remove_D2(int x, int y, int z);
      void remove_vertex(int v);
      void add_edge(int a, int b);
      void contract_edge(int a, int b);
      std::unordered_set<int> check15(int u);
      std::unordered_set<int> R6(int w, int v);
      void add_S_to_q(int a, int b);
      std::unordered_set<int> R7(int u);
      bool equals(std::vector<int>nv, int a, int b, int c);
      std::unordered_set<int> applyR8 (int v1, int v2, int v3, int u1, int u2, int w1, int w2);
      int the_other(int x, std::pair<int, int>& p);
      std::unordered_set<int> R8(int x);
      std::unordered_set<int> applyR9 (int u1, int u2, int u3, int u4, int w1, int w2);
      std::unordered_set<int> R9(int x);
      bool at_most_one_edge(int x, std::set<int> s);
      std::unordered_set<int> applyR101(int v1, int v2, int u, int w1, int w2);
      std::unordered_set<int> R101(int x);
      std::unordered_set<int> checkR102(int u1, int u2, int u3, int w1, int w2);
      std::unordered_set<int> R102(int x);
      std::unordered_set<int> applyR11(int u, int v, int v1, int v2, int v3, int v4, int v5, int v6, int w1, int w2);
      std::unordered_set<int> R11(int x);
  
  
  
  
  
    public:
          
      FVS_kernel(std::list<vertex>& X); // initialize input graph
      std::list<vertex> compute_kernel(); 
      std::list<vertex> get_kernel() { return G; }
      int get_kernel_size() { return G.size(); }
      std::unordered_set<int> get_opt() { return opt; }
      void recover_solution(bool minimal=true); // lifting step, a post-processing for kernel to convert solution to original input graph
      int get_greedy_size() {return greedy.size(); }
      void add_solution(std::vector<int> newset); // add newset to internal solution set
      void add_solution(std::unordered_set<int> newset); // add newset to internal solutions set
      void rDivision(rDiv& div, int r); // compute a r-division for G
  
      // add boundary vertices to set bound and opt
      void add_boundary(std::vector<int>& b) {  for (auto& v: b) { bound.insert(v); opt.insert(v);} }

      // combine 2-approx with reduction rules
      // greedy choose high degree vertices and apply reduction rules after each choice
      // this will modify internal data structures
      void approximate();
      // set step size and lower bound flag
      void set_step_size(int s, bool lower=false) { step_size = s; lower_bound=lower;}

      // obtain lower bound of FVS, which is the number of verties collected by reduction rules 
      int get_lower_bound() { return temp_opt.size() + opt.size() - greedy.size(); }

  };
  
  // generate edge list from file
  std::list< std::vector<int> > generate_edges(std::string file);
  
  // construct a graph from a list of edges
  std::list<vertex> construct(std::list<std::vector<int> >& edges);

  // construct a simple graph from a file containing adjacency lists for all vertices
  std::list<vertex> construct(std::string f);
  
  // compute exactly FVS for a graph 
  std::vector<int> compute_FVS(std::list<vertex>& G);
  std::vector<int> compute_FVS_FPT(std::list<vertex>& G); // by external FPT implementation
  
  
  // test if set s is a FVS for graph H
  bool is_FVS(std::list<vertex>& H, indices& ids, std::unordered_set<int> s);
  
  
  // compute FVS by optimized PTAS: recursively apply kernelization to each level of decomposition until the region is smaller than the given size
  // if the boolean value bound is true, then the size of the boundary is appended to the end of returning vector
  std::vector<int> recurse_kernel_FVS(std::list<vertex>& G, int region_size=20, bool bound=false);
  
  // compute FVS by original PTAS: apply kernel then r-division, then exact algorithm
  // if minimal is false, the solution is not minimal
  std::vector<int> ptas_FVS(std::list<vertex>& G, int region_size=20, bool minimal=true);

  // apply local search 
  std::vector<int> local_search(std::list<vertex>& X, std::vector<int>& fvs, int size, int iterations, std::vector<int>& improvement, unsigned seed=41);


  // set parameters
  void set_path(std::string path);
  void set_time_limit(int seconds);

  // config variables
  extern const std::string fvs_path; // path of FPT algorithm
  extern const int time_limit; // time limit for FPT algorithm 
  extern const std::string temp_input;  //for FPT
  extern const std::string temp_output;// for FPT 

}  

#endif
