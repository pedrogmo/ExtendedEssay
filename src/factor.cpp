#include <algorithm>
#include <chrono>
#include "graph.hpp"

int main(int argc, char **argv)
{
	if (argc != 6)
	{
		std::cerr << "5 arguments expected: file_input, lat1, lon1, lat2, lon2";
        return EXIT_FAILURE;
	}

	Graph graph(argv[1]);
	Graph::id_t v1 = graph.from_location({std::atof(argv[2]), std::atof(argv[3])});
    Graph::id_t v2 = graph.from_location({std::atof(argv[4]), std::atof(argv[5])});
    std::map<Graph::id_t, Graph::id_t> came_from;

    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    bool found = graph.dijkstra(v1, v2, came_from);
    std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = stop - start;
    double time_dijkstra = duration.count();
    std::cout << "Dijsktra took " << time_dijkstra << "s" << std::endl << std::endl;

	if (!found)
    {
        std::cerr << "The path was not found (dijkstra).";
        return EXIT_FAILURE;
    }

	const std::vector<Graph::id_t> path_dijkstra = graph.reconstruct_path(v1, v2, came_from);

    bool paths_equal = false;

    graph.factor = 14.5;

    do
    {
    	graph.factor -= 0.5;

		start = std::chrono::high_resolution_clock::now();
	    found = graph.astar(v1, v2, came_from);
	    stop = std::chrono::high_resolution_clock::now();

	    duration = stop - start;

	    if (!found)
	    {
	        std::cerr << "The path was not found (astar).";
	        return EXIT_FAILURE;
	    }

	    //found
	    std::vector<Graph::id_t> path_astar = graph.reconstruct_path(v1, v2, came_from);

	    paths_equal = std::equal(path_astar.begin(), path_astar.end(), 
	    	path_dijkstra.begin(), path_dijkstra.end());

	    std::cout << "Factor " << graph.factor << " took " << duration.count() << "s" << std::endl;
	    
	}
	while(!paths_equal && graph.factor > 0.0);

	if (graph.factor <= 0.0)
		std::cout << "Negative";
	else
		std::cout << "FACTOR: " << graph.factor;

	return EXIT_SUCCESS;
}