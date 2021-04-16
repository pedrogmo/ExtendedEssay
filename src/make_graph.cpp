#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/haversine.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/all.hpp>

#include "data.hpp"

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
    Data data;
    std::map<osmium::object_id_type, std::uint32_t> link_counter;

public:
    Function function;

    Handler(const char* file) : index_pos(), index_neg(),
        location_handler_type(index_pos, index_neg), 
    	file_out(file), data(), link_counter(),
        function(Function::CountNodes)
    {}

    void way(const osmium::Way& way) 
    {
        const osmium::TagList& tags = way.tags();
        const osmium::WayNodeList& nodelist = way.nodes();

        if(tags.has_key("highway") && !tags.has_tag("highway", "path"))
        {
            if(function == Function::CountNodes)
            {
                for(const osmium::NodeRef& node : nodelist)
                {
                    const osmium::object_id_type ref = node.ref();
                    link_counter[ref]++;

                    if (link_counter[ref] == 2u) //vertex of the graph
                    {
                        //then add location to graph
                        const osmium::Location loc = get_node_location(ref);
                        data.add_location(ref, {loc.lat(), loc.lon()});
                    }
                }
            }

            //first location not added, first not 2u
            else //if(function == Function::CreateData)
            {
                const bool oneway = tags.has_tag("oneway", "true");
                const osmium::NodeRef *first = nullptr, *prev = nullptr;
                double total_length = 0.0;

                for (osmium::WayNodeList::const_iterator it = nodelist.cbegin();
                    it != nodelist.cend(); ++it)
                {
                    const osmium::NodeRef& node = *it;

                    if (!first)
                    {
                        if (link_counter[node.ref()] > 1u)
                        {
                            //then set first to be this node and begin edge
                            first = &node;
                            prev = first;
                        }
                        continue;
                    }

                    const osmium::Location l1 = get_node_location(prev->ref());
                    const osmium::Location l2 = get_node_location(node.ref());

                    total_length = osmium::geom::haversine::distance(
                        osmium::geom::Coordinates(l1), 
                        osmium::geom::Coordinates(l2));

                    if (link_counter[node.ref()] > 1u)
                    {
                        //construct an edge
                        data.add_edge(first->ref(), {node.ref(), total_length}, oneway);
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
        data.output_binary(file_out);
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
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}