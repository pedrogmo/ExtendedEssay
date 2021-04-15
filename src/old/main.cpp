#include "graph.hpp"

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseMemMap<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;

typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

class Handler : public location_handler_type
{
private:
    std::map<osmium::object_id_type, std::uint32_t> link_counter;
    index_pos_type index_pos;
    index_neg_type index_neg;

public:

    Graph graph;

    Handler() : index_pos(), index_neg(),
        location_handler_type(index_pos, index_neg), 
    	link_counter(), graph()
    {}

    void way(const osmium::Way& way) 
    {
        const osmium::TagList& tags = way.tags();
        if (tags.has_key("highway")) //or something else here
        {
            const osmium::NodeRef *prev = &(way.nodes().front());

            for (osmium::WayNodeList::const_iterator it = way.nodes().cbegin() + 1;
                it != way.nodes().cend(); ++it)
            {
                const osmium::NodeRef& node = *it;

        		const osmium::Location l1 = get_node_location(prev->ref());
        		const osmium::Location l2 = get_node_location(node.ref());

                //store locations of nodes
                graph.add_location(prev->ref(), l1);
                graph.add_location(node.ref(), l2);

                const double length = osmium::geom::haversine::distance(
                    osmium::geom::Coordinates(l1), 
                    osmium::geom::Coordinates(l2));

                //construct an edge
                graph.add_edge(prev->ref(), node.ref(), length);

                prev = &node;
            }
        }
    }

}; 

int main(int argc, char **argv) 
{
    if (argc != 7) 
    {
        std::cerr << "7 arguments expected: file, dijkstra/astar, lat1, lon1, lat2, lon2.";
        return EXIT_FAILURE;
    }

    try 
    {
        Handler hnd;
        osmium::io::Reader reader(argv[1], osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
        osmium::apply(reader, hnd);
        Graph& graph = hnd.graph;
    
        Graph::Vertex v1 = graph.from_latlong(std::atof(argv[3]), std::atof(argv[4]));
        Graph::Vertex v2 = graph.from_latlong(std::atof(argv[5]), std::atof(argv[6]));
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
        else
        {
            std::cerr << "Enter either \"dijkstra\" or \"astar\" as 2nd argument.";
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

        /*std::vector<Graph::Vertex> path = graph.reconstruct_path(v1, v2, came_from);

        for(std::uint32_t i = 0; i < path.size(); ++i)
        {
            std::cout << path[i];
            if (i < path.size() - 1)
                std::cout << " -> ";
        }
        std::cout << std::endl;*/
        
        return EXIT_SUCCESS;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}