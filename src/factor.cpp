#include <algorithm>
#include "graph.hpp"

int main(int argc, char **argv)
{
	if (argc != 6)
	{
		std::cerr << "5 arguments expected: file_input, lat1, lon1, lat2, lon2";
        return EXIT_FAILURE;
	}

	Graph graph(argv[1]);
	Graph::Vertex v1 = graph.from_location({std::atof(argv[2]), std::atof(argv[3])});
    Graph::Vertex v2 = graph.from_location({std::atof(argv[4]), std::atof(argv[5])});
    std::map<Graph::Vertex, Graph::Vertex> came_from;

    bool found = graph.dijkstra(v1, v2, came_from);
	if (!found)
    {
        std::cerr << "The path was not found (dijkstra).";
        return EXIT_FAILURE;
    }

	const std::vector<Graph::Vertex> path_dijkstra = graph.reconstruct_path(v1, v2, came_from);

    bool paths_equal = false;

    do
    {
    	graph.factor -= 1.0;
    	std::cout << graph.factor << std::endl;

	    bool found = graph.astar(v1, v2, came_from);

	    if (!found)
	    {
	        std::cerr << "The path was not found (astar).";
	        return EXIT_FAILURE;
	    }

	    //found
	    std::vector<Graph::Vertex> path_astar = graph.reconstruct_path(v1, v2, came_from);

	    paths_equal = std::equal(path_astar.begin(), path_astar.end(), 
	    	path_dijkstra.begin(), path_dijkstra.end());
	}
	while(!paths_equal && graph.factor > 0.0);

	if (graph.factor <= 0.0)
		std::cout << "Negative";
	else
		std::cout << "FACTOR: " << graph.factor;

	return EXIT_SUCCESS;
}