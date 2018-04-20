/********************************************************************
Copyright 2018 Baigong Zheng

********************************************************************/

#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<string>
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
#include<ctime>
#include"FVS_kernel.h"



int main(int argc, char** argv) {
    std::string config_file(argv[1]);
    time_t now = time(0); 
    char* dt = ctime(&now); 
    std::cout << "time " << dt << std::endl;

    std::ifstream config;
    config.open(config_file);

    std::string temp;
    int alg = 4;
    int region_size = 60;
    std::string inputs = "";
    std::string outputs = "";
    bool write_output = true;
    bool test_result = true;
    int frequency = 41;
    int t_start = 3;
    int t_end = 20;
    int t_increase = 3;
    bool show_improve = true;
    int times = 15;
    unsigned seed = 41;
    int terminal = 6;
    while (std::getline(config, temp, '=')) {
      temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
      std::string key = temp;
      std::getline(config, temp);
      temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
      std::string value = temp;
      if (key == "algorithm") 
        alg = std::stoi(value);
      else if (key == "region_size") 
        region_size = std::stoi(value);
      else if (key == "input_file") 
        inputs = value;
      else if (key == "output_file") 
        outputs = value;
      else if (key == "write_output") 
        write_output = (std::stoi(value) == 1);
      else if (key == "test_result") 
        test_result = (std::stoi(value) == 1);
      else if (key == "frequency") 
        frequency = std::stoi(value);
      else if (key == "t_start") 
        t_start = std::stoi(value);
      else if (key == "t_end") 
        t_end = std::stoi(value);
      else if (key == "t_increase") 
        t_increase = std::stoi(value);
      else if (key == "show_improve") 
        show_improve = (std::stoi(value) == 1);
      else if (key == "fvs_path") 
        fvs_kernel::set_path(value); 
      else if (key == "time_limit") 
        times = std::stoi(value);
      else if (key == "random_seed") 
        seed = std::stoi(value);
      else if (key == "terminal") 
        terminal = std::stoi(value); 
      else {
        std::cout << "parameter unknown" << std::endl;
        std::cout << "Please edit the configuration file" << std::endl;
        return 0;
      }
    }
    config.close();

    std::cout << std::endl << "graph: " << inputs << std::endl; // graph name

    // a new graph X
    std::list<fvs_kernel::vertex> X;

    // construct adjacency list for X 
    if (inputs.substr(inputs.size()-3) == ".ir") {
      // from a file containing adjacency lists
      X = fvs_kernel::construct(inputs);
    }
    else {
      // from a file containing edges
      std::list<std::vector<int> > edges = fvs_kernel::generate_edges(inputs); 
      X = fvs_kernel::construct(edges);
    }

    int s = 0;
    for (auto& v: X) {
      for (auto& e: v.edges)
        s += e.multi;
    }

    std::cout << "original graph edge set size: " << s/2 << std::endl; 
    std::cout << "original graph vertex set size: " << X.size() << std::endl;



    std::unordered_set<int> solution;
      
    if (alg == 0) {
      // apply exact algorithm for FVS
      std::cout << "exact algorithm applied" << std::endl;
      fvs_kernel::FVS_kernel K(X);
      K.compute_kernel();
      std::list<fvs_kernel::vertex> kernel_g = K.get_kernel();
      std::vector<int> res = fvs_kernel::compute_FVS_FPT(kernel_g);
      K.add_solution(res);
      K.recover_solution();
      solution = K.get_opt();
    }   

    if (alg == 1) {
      // apply 2-approx algorithm
      std::cout << "2-approximation algorithm applied" << std::endl;
      fvs_kernel::Approx_FVS appx(X);
      std::vector<int> app_res = appx.approximate(); 
      solution.insert(app_res.begin(), app_res.end()); 
    }

    if (alg == 2) {
      // apply kernel and then 2-approx algorithm 
      std::cout << "kernel and 2-approximation algorithm applied" << std::endl;
      fvs_kernel::FVS_kernel K(X); 
      K.compute_kernel();
      std::list<fvs_kernel::vertex> kernel_g = K.get_kernel();
      std::cout << "kernel size: " << kernel_g.size() << std::endl;
      fvs_kernel::Approx_FVS appx_k(kernel_g);
      std::vector<int> res = appx_k.approximate();
      K.add_solution(res);
      K.recover_solution();
      solution = K.get_opt(); 
    }


    if (alg == 3) {
      // apply hybrid algorithm
      std::cout << "hybrid algorithm applied" << std::endl;
      fvs_kernel::FVS_kernel K(X); 
      K.compute_kernel();
      K.set_step_size(frequency);
      K.approximate();
      K.recover_solution();
      solution = K.get_opt(); 
    }

    if (alg == 4) {
      // apply Heuristic Approximation Scheme
      std::cout << "Heuristic Approximation Scheme applied" << std::endl;
      
      // compute a kernel for original graph
      fvs_kernel::FVS_kernel K(X); 
      K.compute_kernel();
      std::list<fvs_kernel::vertex> kernel = K.get_kernel();

      // copy kernel to G
      std::list<fvs_kernel::vertex> G;
      for (auto& v: kernel) 
        G.push_back(fvs_kernel::vertex(v));

      // apply HAS on G and then lift the solution
      // apply hybrid algorithm first
      fvs_kernel::FVS_kernel KK(G);
      KK.compute_kernel();
      KK.set_step_size(frequency);
      KK.approximate(); 
      KK.recover_solution();

      std::vector<int> fvs, fvs1;
      // solution from hybrid algorithm 
      fvs.insert(fvs.end(), KK.get_opt().begin(), KK.get_opt().end());
  
      // apply local search
      for (int t = t_start; t <= t_end; t += t_increase) { 
        int size = fvs.size()/t;
        int iterations = -terminal; 
        std::vector<int> improvement;
        fvs1 = fvs_kernel::local_search(G, fvs, size, iterations, improvement, seed);
        if (fvs.size() > fvs1.size())
          fvs = fvs1;
        if (show_improve) {
          std::cout << "improvement for each search: " << std::endl;
          for (auto& ip: improvement)
            std::cout << ip << " ";
          std::cout << std::endl;
        } 
      } 
  
      // add solution back to original graph
      K.add_solution(fvs); 
      K.recover_solution();
    
      solution = K.get_opt(); 
    }    

    if (alg == 5) { 
      // apply PTAS without any optimization 
      std::cout << "vanilla PTAS applied" << std::endl;
      std::vector<int> ptas_res = fvs_kernel::ptas_FVS(X, region_size, false); 
      solution.insert(ptas_res.begin(), ptas_res.end()); 
    }

    if (alg == 6) { 
      // apply PTAS with post-processing step
      std::cout << "PTAS with post-processing applied" << std::endl;
      std::vector<int> ptas_res = fvs_kernel::ptas_FVS(X, region_size); 
      solution.insert(ptas_res.begin(), ptas_res.end()); 
    }

    if (alg == 7) { 
      // apply optimized PTAS with both heuristics
      std::cout << "optimized PTAS with both heuristics applied" << std::endl;
      std::vector<int> ptas_res = fvs_kernel::recurse_kernel_FVS(X, region_size); 
      solution.insert(ptas_res.begin(), ptas_res.end()); 
    } 


    std::cout << "final solution has size: " << solution.size() << std::endl; 

    if (test_result) {
      // test if solution is FVS
      fvs_kernel::indices ids; 
      std::list<fvs_kernel::vertex>::iterator it;
      for (it = X.begin(); it != X.end(); it++)
        ids[it->id] = it;
      std::cout <<"this solution is FVS: " << fvs_kernel::is_FVS(X, ids, solution) << std::endl;
      }

    if (write_output) {
      std::ofstream f;
      f.open(outputs); 
      for (auto& v: solution)
        f << v << std::endl;
      f.close();
    }


    now = time(0); 
    dt = ctime(&now); 
    std::cout << "time " << dt << std::endl;


    return 0;
}


