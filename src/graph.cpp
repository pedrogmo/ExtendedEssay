#include <limits>
#include <cmath>
#include "graph.hpp"

constexpr double Graph::EARTH_RADIUS_KM;

inline bool Graph::greater_pqelement::operator() (
	const Graph::PQElement &x, const Graph::PQElement &y) const 
{
	return x.first > y.first; 
}

inline double Graph::degree_to_radian(double angle) const
{
	return M_PI * angle / 180.0;
}

inline Graph::cost_t Graph::heuristic(const Graph::Vertex &v1, const Graph::Vertex &v2) const 
{
	const Location &l1 = v1.loc;
	const Location &l2 = v2.loc;

	const double lat_rad1 = degree_to_radian(l1.lat);
	const double lat_rad2 = degree_to_radian(l2.lat);
	const double lon_rad1 = degree_to_radian(l1.lon);
	const double lon_rad2 = degree_to_radian(l2.lon);
 
	const double diff_lat = lat_rad2 - lat_rad1;
	const double diff_lon = lon_rad2 - lon_rad1;
 
	const double computation = std::asin(std::sqrt(std::sin(diff_lat / 2) * std::sin(diff_lat / 2) + 
		std::cos(lat_rad1) * std::cos(lat_rad2) * std::sin(diff_lon / 2) * std::sin(diff_lon / 2)));

	return 2.0 * EARTH_RADIUS_KM * computation / 35.0;
}

Graph::Graph()
: vertices()
{}

Graph::Graph(const char *filename)
{
	std::ifstream in(filename, std::ios::binary);

	in.read(reinterpret_cast<char*>(&n_vertices), sizeof(n_vertices));

	std::multimap<id_t, Connection> connections;

	for(std::size_t n = 0u; n < n_vertices; ++n)
	{
		Vertex vertex;
		
		in.read(reinterpret_cast<char*>(&vertex.id), sizeof(vertex.id));
		in.read(reinterpret_cast<char*>(&vertex.loc), sizeof(vertex.loc));

		std::size_t n_connections = 0u;
		in.read(reinterpret_cast<char*>(&n_connections), sizeof(n_connections));

		for(; n_connections > 0; --n_connections)
		{
			Connection conn;
			in.read(reinterpret_cast<char*>(&conn), sizeof(conn));
			connections.insert({vertex.id, conn});
		}

		vertices.insert({vertex.id, vertex});
	}

	//now traverse connections to set pointers to vertices
	for(auto it = connections.cbegin(); it != connections.cend(); ++it)
	{
		Edge e = {&vertices.at(it->second.id), it->second.cost};
		vertices.at(it->first).edges.push_back(e);
		n_edges++;
	}
}

void Graph::add_vertex(Graph::id_t vertex_id, Graph::Location location)
{
	Vertex v;
	v.id = vertex_id;
	v.loc = location;

	vertices[vertex_id] = v;
	n_vertices++;
}

void Graph::add_edge(Graph::id_t from_id, Graph::id_t to_id, 
	Graph::cost_t cost, bool one_directional) 
{
	Edge e = {&vertices.at(to_id), cost};
	vertices.at(from_id).edges.push_back(e);
	n_edges++;

	if (!one_directional)
	{
		//reverse: to_id -> from_id
		Edge back = {&vertices.at(from_id), cost};
		vertices.at(to_id).edges.push_back(back);
		n_edges++;
	}
}

inline std::size_t Graph::vertex_count() const noexcept
{
	return n_vertices;
}

inline std::size_t Graph::edge_count() const noexcept
{
	return n_edges;
}

Graph::id_t Graph::from_location(Graph::Location target) const
{
	id_t closest_match;
	double minimum_difference = std::numeric_limits<double>::max();

	for(auto it = vertices.cbegin(); it != vertices.cend(); ++it)
	{
		const Location l = it->second.loc;
		const double diff = std::abs(target.lat - l.lat) + std::abs(target.lon - l.lon);

		if (diff < minimum_difference)
		{
			closest_match = it->first;
			minimum_difference = diff;
		}
	}

	return closest_match;
}

Graph::Location Graph::location(Graph::id_t vertex_id) const
{
	return vertices.at(vertex_id).loc;
}

std::ostream& operator<<(std::ostream &out, const Graph::Location &l)
{
	out << l.lat << ", " << l.lon;
	return out;
}

std::ostream& operator<<(std::ostream &out, const Graph &data)
{
	for(auto it = data.vertices.cbegin(); it != data.vertices.cend(); ++it)
	{
		out << it->first << ": " << it->second.loc << std::endl << "{";
		for(const Graph::Edge &e : it->second.edges)
		{
			out << "(" << e.destination->id << ", " << e.cost << "), ";
		}
		out << "}" << std::endl;
	}

	return out;
}

void Graph::output_binary(const char *filename)
{
	std::ofstream out(filename, std::ios::binary);
	out.write(reinterpret_cast<const char*>(&n_vertices), sizeof(n_vertices));
	std::map<id_t, Connection> connections;

	for(auto it = vertices.cbegin(); it != vertices.cend(); ++it)
	{
		const Vertex &vertex = it->second;
		const std::size_t n_connections = vertex.edges.size();

		out.write(reinterpret_cast<const char*>(&vertex.id), sizeof(vertex.id));
		out.write(reinterpret_cast<const char*>(&vertex.loc), sizeof(vertex.loc));

		out.write(reinterpret_cast<const char*>(&n_connections), sizeof(n_connections));

		for(const Edge &e : vertex.edges)
		{
			Connection conn = {e.destination->id, e.cost};
			out.write(reinterpret_cast<const char*>(&conn), sizeof(conn));
		}
	}
}

bool Graph::dijkstra(Graph::id_t start_id, Graph::id_t goal_id,
	std::map<Graph::id_t, Graph::id_t> &came_from, size_t &nodes_visited) const
{
	std::map<id_t, cost_t> cost_so_far;
	PriorityQueue frontier;

	frontier.push(PQElement(0.0, &vertices.at(start_id)));
	//came_from[start_id] = start_id;
	cost_so_far[start_id] = 0.0;

	while (!frontier.empty()) 
	{
		const Vertex *current = frontier.top().second;
		frontier.pop();

		++nodes_visited;

		if (current->id == goal_id) 
		{
			return true;
		}

		for(const Edge &edge : current->edges)
		{
			const cost_t new_cost = cost_so_far[current->id] + edge.cost;

			//if cost does not exist or new_cost is smaller, update cost
			std::map<id_t, cost_t>::iterator it = cost_so_far.find(edge.destination->id);
			if (it == cost_so_far.end() ||
				new_cost < it->second) 
			{
				cost_so_far[edge.destination->id] = new_cost;
				came_from[edge.destination->id] = current->id;
				frontier.push(PQElement(new_cost, edge.destination));
			}
		}
	}

	return false;
}

bool Graph::astar(Graph::id_t start_id, Graph::id_t goal_id,
	std::map<Graph::id_t, Graph::id_t> &came_from, size_t &nodes_visited) const
{
	std::map<id_t, cost_t> cost_so_far;
	PriorityQueue frontier;
	const Vertex &goal = vertices.at(goal_id);

	frontier.push(PQElement(0.0, &vertices.at(start_id)));
	//came_from[start_id] = start_id;
	cost_so_far[start_id] = 0.0;

	while (!frontier.empty()) 
	{
		const Vertex *current = frontier.top().second;
		frontier.pop();

		++nodes_visited;

		if (current->id == goal_id) 
		{
			return true;
		}

		for(const Edge &edge : current->edges)
		{
			const cost_t new_cost = cost_so_far[current->id] + edge.cost;

			//if cost does not exist or new_cost is smaller, update cost
			std::map<id_t, cost_t>::iterator it = cost_so_far.find(edge.destination->id);
			if (it == cost_so_far.end() ||
				new_cost < it->second) 
			{
				cost_so_far[edge.destination->id] = new_cost;
				const cost_t priority = new_cost + heuristic(*edge.destination, goal);
				came_from[edge.destination->id] = current->id;
				frontier.push(PQElement(priority, edge.destination));
			}
		}
	}

	return false;
}

std::vector<Graph::id_t> Graph::reconstruct_path(Graph::id_t start_id, Graph::id_t goal_id,
	std::map<Graph::id_t, Graph::id_t> &came_from) const
{
	std::vector<id_t> c;
	
	for(id_t v = goal_id; v != start_id; v = came_from[v])
	{
		c.push_back(v);
	}

	c.push_back(start_id);
	std::reverse(c.begin(), c.end());
	
	return c;
}