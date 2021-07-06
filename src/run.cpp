#include <chrono>
#include "graph.hpp"

int main(int argc, char **argv)
{
	if (argc != 7 && argc != 8)
	{
		std::cerr << "6/7 arguments expected: file_input, dijkstra/astar/locate, lat1, lon1, lat2, lon2, (optional : file_output_kml)";
        return EXIT_FAILURE;
	}

	Graph graph(argv[1]);
    graph.factor = 14.0;
	Graph::id_t v1 = graph.from_location({std::atof(argv[3]), std::atof(argv[4])});
    Graph::id_t v2 = graph.from_location({std::atof(argv[5]), std::atof(argv[6])});
    std::map<Graph::id_t, Graph::id_t> came_from;

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
    std::cout << duration.count() << "s" << std::endl;

    if (argc == 8) //extra argument: output kml
    {
        std::vector<Graph::id_t> vlist = graph.reconstruct_path(v1, v2, came_from);

        std::ofstream kml_out(argv[7]);
        kml_out << 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" <<
        "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n" <<
        "<Document>\n" <<
          "<name>LineStyle.kml</name>\n" <<
          "<open>1</open>\n" <<
          "<Style id=\"linestyleExample\">\n" <<
            "<LineStyle>\n" <<
              "<color>" << (dijkstra_or_astar ? "7f0000ff" : "7fff0000") << "</color>\n" <<
              "<width>4</width>\n" <<
              "<gx:labelVisibility>1</gx:labelVisibility>\n" <<
            "</LineStyle>\n" <<
          "</Style>\n" <<
          "<Placemark>\n" <<
            "<name>LineStyle Example</name>\n" <<
            "<styleUrl>#linestyleExample</styleUrl>\n" <<
            "<LineString>\n" <<
              "<extrude>1</extrude>\n" <<
              "<tessellate>1</tessellate>\n" <<
              "<coordinates>\n";
        
        for(const Graph::id_t& v : vlist)
        {
            const Graph::Location l = graph.location(v);
            kml_out << l.lon << "," << l.lat << ",0" << std::endl;
        }
        const Graph::Location l = graph.location(vlist.front());
        kml_out << l.lon << "," << l.lat << ",0" << std::endl;

        kml_out << 
        "</coordinates>\n" <<
        "</LineString>\n" <<
          "</Placemark>\n" <<
        "</Document>\n" <<
        "</kml>";

        kml_out.close();
    }

	return EXIT_SUCCESS;
}