/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "planargraph.h"
#include "r_divisor.h"


// the string split function is copied from https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
void split(std::vector<std::string> & theStringVector,  /* Altered/returned value */
	const  std::string  & theString,
	const  std::string  & theDelimiter)
{

	size_t  start = 0, end = 0;

	while (end != std::string::npos)
	{
		end = theString.find(theDelimiter, start);

		// If at end, use length=maxLength.  Else use length=end-start.
		theStringVector.push_back(theString.substr(start,
			(end == std::string::npos) ? std::string::npos : end - start));

		// If at end, use start=maxSize.  Else use start=end+delimiter.
		start = ((end > (std::string::npos - theDelimiter.size()))
			? std::string::npos : end + theDelimiter.size());
	}
}

// set buffer to speed up read
//http://en.cppreference.com/w/cpp/io/basic_filebuf/setbuf
void read_embedding_from_file(char *filename, std::vector<std::vector<int>> &embedding_storage) {
	// the embedding output from John M. Boyer's program is counter-clockwise 
	// while our input embedding is clock-wise, need to revert the embedding while reading
	std::cout << "Read planar embedding from file" << std::endl;
	time_t begin, end;
	time(&begin);
	char buf[1000000];
	std::ifstream infile;
	infile.rdbuf()->pubsetbuf(buf, sizeof buf);
	infile.open(filename);
	std::string line;
	std::getline(infile, line);
	std::vector<std::string> chunks;

	split(chunks, line, "=");
	int n = std::stoi(chunks[1]);
	embedding_storage.reserve(n);
	std::vector<int> adj_list;
	int line_cnt = 0;
	while (std::getline(infile, line))
	{
		adj_list.clear();
		chunks.clear();
		split(chunks, line, " ");
		for (int i = 1; i < chunks.size() - 1; i++) {
			adj_list.push_back(std::stoi(chunks[chunks.size() - i - 1]));
		}
		embedding_storage.push_back(adj_list);
		if (line_cnt % 100000 == 0) {
			printf("line count %d\n", line_cnt);
		}
		line_cnt++;
	}
	time(&end);
	double difference = difftime(end, begin);
	std::cout << "Total time for reading is " << difference << " seconds." << std::endl;
}

void write_output(std::vector<int> &boundary_vertices, std::list<planargraph> &small_graph_lists, char *outputfilename) {
	std::cout << "Write output to "<< outputfilename << std::endl;
	time_t begin, end;
	time(&begin);
	std::ofstream outputfile(outputfilename);
	// write the separator
	outputfile << "#0" << std::endl;
	outputfile << "N=" << boundary_vertices.size() << std::endl;
	for (int i = 0; i < boundary_vertices.size()-1; i++) {
		outputfile << boundary_vertices[i] << " ";
	}
	outputfile << boundary_vertices[boundary_vertices.size() - 1] << std::endl;
	int i = 1;
	while (!small_graph_lists.empty()) {
		outputfile << "#" << i << std::endl;
		outputfile << "N=" << small_graph_lists.back().n << " M=" << small_graph_lists.back().m << std::endl;
		small_graph_lists.back().write_to_file(outputfile);
		small_graph_lists.pop_back();
		i++;
	}
	outputfile.close();
	time(&end);
	double difference = difftime(end, begin);
	std::cout << "Total time for writing output is " << difference << " seconds." << std::endl;
	// write component
}
void print_separator(std::vector<int> &vec) {
	std::cout << "Separator size" << vec.size() << std::endl;
	for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << "\t" << std::endl;
	}
	std::cout << std::endl;
}


/**********************************************************
*****
*****	GRAPH IO
*****
***********************************************************
*/
void write_division_from_input_embedding(int argc, char *argv[]) {
	std::vector<std::vector<int>> embedding_storage;
	read_embedding_from_file(argv[1], embedding_storage);
	int r = std::stoi(argv[3]);
	planargraph g(embedding_storage.size(), embedding_storage);
	std::vector<int> boundary_vertices;
	std::list<planargraph> small_graph_lists;
	r_division(g, r, boundary_vertices, small_graph_lists);
	write_output(boundary_vertices, small_graph_lists, argv[2]);
}
