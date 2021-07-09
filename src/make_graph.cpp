#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/haversine.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/all.hpp>

#include <cstring>

#include "graph.hpp"

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseMemMap<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;

typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

class Handler : public location_handler_type
{
public:
    enum class Function
    {
        CountNodes, CreateData
    };
private:
    index_pos_type index_pos;
    index_neg_type index_neg;
    std::ofstream file_out;
    Graph graph;
    std::map<osmium::object_id_type, std::uint32_t> link_counter;

public:
    Function function;

    Handler(const char* file) : 
        index_pos(), index_neg(),
        location_handler_type(index_pos, index_neg), 
    	file_out(file, std::ios::binary), graph(), 
        link_counter(),
        function(Function::CountNodes)
    {}

    void way(const osmium::Way &way) 
    {
        const osmium::TagList &tags = way.tags();
        const osmium::WayNodeList &nodelist = way.nodes();

        if(tags.has_key("highway") && 
            !tags.has_tag("highway", "path") && 
            !tags.has_tag("highway", "pedestrian"))
        {
            if(function == Function::CountNodes)
            {
                for(const osmium::NodeRef &node : nodelist)
                {
                    const osmium::object_id_type ref = node.ref();
                    link_counter[ref]++;

                    if (link_counter[ref] == 2u) //vertex of the graph
                    {
                        //then add location to graph
                        const osmium::Location loc = get_node_location(ref);
                        graph.add_vertex(ref, {loc.lat(), loc.lon()});
                    }
                }
            }

            //first location not added, first not 2u
            else //if(function == Function::CreateData)
            {
                const bool oneway = tags.has_tag("oneway", "true");
                const osmium::NodeRef *first = nullptr, *prev = nullptr;
                double total_length = 0.0;
                
                const char *speed_str = tags.get_value_by_key("maxspeed");
                double speed = 30.0; //default speed in km/h
                if (!speed_str) //not provided (majority)
                {
                    if (tags.has_tag("highway", "motorway"))
                        speed = 120.0;
                    else if (tags.has_tag("highway", "motorway_link"))
                        speed = 120.0;
                    else if (tags.has_tag("highway", "trunk"))
                        speed = 100.0;
                    else if (tags.has_tag("highway", "trunk_link"))
                        speed = 100.0;
                    else if (tags.has_tag("highway", "primary"))
                        speed = 90.0;
                    else if (tags.has_tag("highway", "primary_link"))
                        speed = 90.0;
                    else if (tags.has_tag("highway", "secondary"))
                        speed = 70.0;
                    else if (tags.has_tag("highway", "secondary_link"))
                        speed = 70.0;
                    else if (tags.has_tag("highway", "tertiary"))
                        speed = 60.0;
                    else if (tags.has_tag("highway", "tertiary_link"))
                        speed = 60.0;
                    else if (tags.has_tag("highway", "unclassified"))
                        speed = 50.0;
                    else if (tags.has_tag("highway", "residential"))
                        speed = 30.0;
                    else if (tags.has_tag("highway", "living_street"))
                        speed = 10.0;
                    else if(tags.has_tag("highway", "service"))
                        speed = 30.0;
                    else if (tags.has_tag("highway", "track"))
                        speed = 30.0;
                }
                else if (const char *ptr = std::strstr(speed_str, "mph"))
                    speed = std::atof(speed_str) * 1.60934;
                else if (const char *ptr = std::strstr(speed_str, "knots"))
                    speed = std::atof(speed_str) * 1.852;
                else if (std::strncmp(speed_str, "none", std::strlen(speed_str)) == 0)
                    speed = 180.0;
                else if (std::strncmp(speed_str, "walk", std::strlen(speed_str)) == 0)
                    speed = 5.0;
                else //speed in km/h
                    speed = std::atof(speed_str);

                for (osmium::WayNodeList::const_iterator it = nodelist.cbegin();
                    it != nodelist.cend(); ++it)
                {
                    const osmium::NodeRef &node = *it;

                    if (!first) //if first is null, check if node should be first
                    {
                        if (link_counter[node.ref()] > 1u)
                        {
                            //then set first to be this node and begin edge
                            first = &node;
                            prev = first;
                        }
                        continue;
                    }

                    //if first is not null, add distance from prev

                    const osmium::Location l1 = get_node_location(prev->ref());
                    const osmium::Location l2 = get_node_location(node.ref());

                    total_length += osmium::geom::haversine::distance(
                        osmium::geom::Coordinates(l1), 
                        osmium::geom::Coordinates(l2));

                    if (link_counter[node.ref()] > 1u)
                    {
                        //construct an edge
                        graph.add_edge(first->ref(), node.ref(), total_length / speed, oneway);
                        total_length = 0.0;
                        first = &node;
                    }
                    
                    prev = &node;
                }
            }
            
        }
    }

    void output()
    {
        graph.output_binary(file_out);
    }

}; 

int main(int argc, char **argv) 
{
    if (argc != 3) 
    {
        std::cerr << "2 arguments expected: file_input, file_output.";
        return EXIT_FAILURE;
    }

    try 
    {
        Handler hnd(argv[2]);
        osmium::io::Reader reader1(argv[1], osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
        osmium::apply(reader1, hnd);

        hnd.function = Handler::Function::CreateData;
        osmium::io::Reader reader2(argv[1], osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
        osmium::apply(reader2, hnd);

        hnd.output();
        
        return EXIT_SUCCESS;
    } 
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}