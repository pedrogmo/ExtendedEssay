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
private:
    index_pos_type index_pos;
    index_neg_type index_neg;
    std::ofstream file_out;
    Data data;

public:

    Handler(const char* file) : index_pos(), index_neg(),
        location_handler_type(index_pos, index_neg), 
    	file_out(file), data()
    {}

    void way(const osmium::Way& way) 
    {
        const osmium::TagList& tags = way.tags();
        //if (tags.has_tag("highway", "road"))
        if(tags.has_key("highway") && !tags.has_tag("highway", "path"))
        {
            const osmium::NodeRef *prev = &(way.nodes().front());

            for (osmium::WayNodeList::const_iterator it = way.nodes().cbegin() + 1;
                it != way.nodes().cend(); ++it)
            {
                const osmium::NodeRef& node = *it;

        		const osmium::Location l1 = get_node_location(prev->ref());
        		const osmium::Location l2 = get_node_location(node.ref());

                //store locations of nodes
                data.add_location(prev->ref(), {l1.lat(), l1.lon()});
                data.add_location(node.ref(), {l2.lat(), l2.lon()});

                const double length = osmium::geom::haversine::distance(
                    osmium::geom::Coordinates(l1), 
                    osmium::geom::Coordinates(l2));

                //construct an edge
                data.add_edge(prev->ref(), {node.ref(), length});

                prev = &node;
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
        osmium::io::Reader reader(argv[1], osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
        osmium::apply(reader, hnd);
        hnd.output();
        
        return EXIT_SUCCESS;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}