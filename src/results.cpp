#include <cstring>
#include "graph.hpp"

#define ROWS 10
#define COLUMNS 2

static void not_found(int, int);

static const Graph::Location coordinates[ROWS][COLUMNS] =
{
    { {52.82129221319522, 1.3866942261891224},{52.771676763963804, 1.515929070363573} },
    { {52.68093254057173, 0.9399893330374438},{52.5767673437174, 1.7275985447676063} },
    { {52.63098052957379, 1.3016297964250163},{52.243552979263455, 0.7164727863826298} },
    { {52.20437220753986, 0.11765769300282693},{52.634021605007895, -1.138283831712465} },
    { {51.70319781568003, -0.024207776154516195},{51.318159080111386, -0.5564664183746294} },
    { {52.40886459469964, -1.5063983662127676},{52.05561452340814, 1.1582299611262965} },
    { {51.750198656386296, -1.2568330856395429},{51.279960941381546, 1.0807164149518576} },
    { {52.924701784417614, -1.478078545086647},{50.81938560342749, -0.12672061916976493} },
    { {53.96202236030352, -1.0836604324091592},{50.8041267858146, -1.0768792542479164} },
    { {50.37390942756683, -4.1515926909224845},{52.879684558443664, 1.432229241948389} },
};

int main(int argc, char **argv)
{
    Graph graph("data/england.dat");
    std::map<Graph::id_t, Graph::id_t> came_from;
    bool found = true;

    for(int i = 0; i < ROWS; ++i)
    {
        const Graph::id_t v1 = graph.from_location(coordinates[i][0]);
        const Graph::id_t v2 = graph.from_location(coordinates[i][1]);

        size_t dijkstra_nodes_visited = 0u;
        size_t astar_nodes_visited = 0u;
        
        found = graph.dijkstra(v1, v2, came_from, dijkstra_nodes_visited);

        if (!found)
            not_found(i, 0);
        else
            std::cout << dijkstra_nodes_visited << '\t';
    
        found = graph.astar(v1, v2, came_from, astar_nodes_visited);

        if (!found)
            not_found(i, 1);
        else
            std::cout << astar_nodes_visited << '\t' << std::endl;
    }
    
	return EXIT_SUCCESS;
}

inline void not_found(int row, int mode)
{
    std::cerr << "Error row " << row << " mode " << mode << '\n';
    std::exit(EXIT_FAILURE);
}