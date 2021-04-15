#include <chrono>
#include "graph.hpp"

int main(int argc, char **argv)
{
	if (argc != 7)
	{
		std::cerr << "6 arguments expected: file_input, dijkstra/astar, lat1, lon1, lat2, lon2.";
        return EXIT_FAILURE;
	}

	Graph graph(argv[1]);
	Graph::Vertex v1 = graph.from_location({std::atof(argv[3]), std::atof(argv[4])});
    Graph::Vertex v2 = graph.from_location({std::atof(argv[5]), std::atof(argv[6])});
    std::map<Graph::Vertex, Graph::Vertex> came_from;

    bool dijkstra_or_astar = true;

    if (std::strncmp(argv[2], "dijkstra", std::strlen(argv[2])) == 0)
    {
        dijkstra_or_astar = true;
    }
    else if (std::strncmp(argv[2], "astar", std::strlen(argv[2])) == 0)
    {
        dijkstra_or_astar = false;
    }
    else if (std::strncmp(argv[2], "locate", std::strlen(argv[2])) == 0)
    {
        Graph::Location l1 = graph.location(v1), l2 = graph.location(v2);
        std::cout << l1.lat << ", " << l1.lon << std::endl <<
            l2.lat << ", " << l2.lon << std::endl;
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr << "Enter either \"dijkstra\" or \"astar\" or \"locate\" as 2nd argument.";
        return EXIT_FAILURE;
    }

    bool found = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

    if (dijkstra_or_astar)
        found = graph.dijkstra(v1, v2, came_from);
    else
        found = graph.astar(v1, v2, came_from);

    std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();

    if (!found)
    {
        std::cerr << "The path was not found.";
        return EXIT_FAILURE;
    }

    //found
    std::chrono::duration<double> duration = stop - start;
    std::cout << duration.count() << "s";

	return EXIT_SUCCESS;
}