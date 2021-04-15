#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "header.hpp"

class Graph
{
public:
	typedef osmium::object_id_type Vertex;
	typedef double Cost;

	struct Edge
	{
		Vertex destination;
		Cost cost;
	};

private:
	typedef std::pair<Cost, Vertex> PQElement;

	struct greater_pqelement
	{
		bool operator() (const PQElement& x, const PQElement& y) const 
		{
			return x.first > y.first; 
		}
	};

	typedef std::priority_queue<PQElement, std::vector<PQElement>, greater_pqelement> PriorityQueue;

	std::multimap<Vertex, Edge> edges;
	std::uint32_t n_vertices;
	std::unordered_map<Vertex, osmium::Location> locations;

	inline double heuristic(Vertex v1, Vertex v2) const
	{
		//maybe use lat() & long() instead of x() & y()
		const osmium::Location a = locations.at(v1), b = locations.at(v2);
		return std::abs(a.x() - b.x()) + std::abs(a.y() - b.y());
	}

public:
	Graph() : edges(), n_vertices(0)
	{}

	void add_edge(Vertex v1, Vertex v2, Cost c)
	{
		if (edges.count(v1) != 0)
			n_vertices++;
		Edge edge = {v2, c};
		edges.emplace(v1, edge);
	}

	void add_location(Vertex v, osmium::Location l)
	{
		locations[v] = l;
	}

	Vertex from_latlong(double lat, double lon) const
	{
		Vertex closest_match;
		double minimum_difference = std::numeric_limits<double>::max();

		for(auto it = locations.cbegin(); it != locations.cend(); ++it)
		{
			Vertex v = it->first;
			osmium::Location l = it->second;
			const double diff = std::abs(lat - l.lat()) + std::abs(lon - l.lon());
			if (diff < minimum_difference)
			{
				closest_match = v;
				minimum_difference = diff;
			}
		}

		return closest_match;
	}

	std::uint32_t node_count() const
	{
		return n_vertices;
	}

	std::uint32_t edge_count() const
	{
		return edges.size();
	}

	friend std::ostream& operator<< (std::ostream& out, const Graph& graph)
	{
		for(auto it = graph.edges.begin(); it != graph.edges.end(); ++it)
		{
			out << it->first << " -> " << it->second.destination << ": " << it->second.cost << std::endl;
		}

		return out;
	}

	bool dijkstra(Vertex start, Vertex goal,
		std::map<Vertex, Vertex>& came_from)
	{
		std::map<Vertex, Cost> cost_so_far;
		PriorityQueue frontier;

		frontier.emplace(0.0, start);

		//came_from[start] = start;
		cost_so_far[start] = 0.0;

		while (!frontier.empty()) 
		{
			Vertex current = frontier.top().second;
			frontier.pop();

			if (current == goal) 
			{
				return true;
			}

			std::pair< std::map<Vertex, Edge>::const_iterator, std::map<Vertex, Edge>::const_iterator > p = edges.equal_range(current);

			for (std::map<Vertex, Edge>::const_iterator it = p.first; it != p.second; ++it) 
			{
				const Edge& edge = it->second;
				const Cost new_cost = cost_so_far[current] + edge.cost;

				if (cost_so_far.find(edge.destination) == cost_so_far.end()
					|| new_cost < cost_so_far[edge.destination]) 
				{
					cost_so_far[edge.destination] = new_cost;
					came_from[edge.destination] = current;
					frontier.emplace(new_cost, edge.destination);
				}
			}
		}

		return false;
	}

	bool astar(Vertex start, Vertex goal,
		std::map<Vertex, Vertex>& came_from)
	{
		std::map<Vertex, Cost> cost_so_far;
		PriorityQueue frontier;

		frontier.emplace(0.0, start);

		//came_from[start] = start;
		cost_so_far[start] = 0.0;

		while (!frontier.empty()) 
		{
			Vertex current = frontier.top().second;
			frontier.pop();

			if (current == goal) 
			{
				return true;
			}

			std::pair< std::map<Vertex, Edge>::const_iterator, std::map<Vertex, Edge>::const_iterator > p = edges.equal_range(current);

			for (std::map<Vertex, Edge>::const_iterator it = p.first; it != p.second; ++it) 
			{
				const Edge& edge = it->second;
				const Cost new_cost = cost_so_far[current] + edge.cost;

				if (cost_so_far.find(edge.destination) == cost_so_far.end()
					|| new_cost < cost_so_far[edge.destination]) 
				{
					cost_so_far[edge.destination] = new_cost;
					double priority = new_cost + heuristic(edge.destination, goal);
					came_from[edge.destination] = current;
					frontier.emplace(priority, edge.destination);
				}
			}
		}

		return false;
	}

	std::vector<Vertex> reconstruct_path(Vertex start, Vertex goal,
		std::map<Vertex, Vertex>& came_from) const
	{
		std::vector<Vertex> c;
		
		for(Vertex v = goal; v != start; v = came_from[v])
		{
			c.push_back(v);
		}

		c.push_back(start);
		std::reverse(c.begin(), c.end());
		
		return c;
	}
};

#endif //GRAPH_HPP