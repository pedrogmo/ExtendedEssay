#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <queue>
#include <cstdint>
#include <cmath>
#include <map>
#include <list>
#include <limits>
#include <iostream>
#include <fstream>

class Graph
{
public:
	typedef double cost_t;
	typedef std::int64_t id_t;

	struct Location
	{
		double lat;
		double lon;
	};

private:
	struct Vertex
	{
		struct Edge
		{
			Vertex *destination;
			cost_t cost;
		};

		id_t id;
		Location loc;
		std::list<Edge> edges;
	};

	struct Connection
	{
		id_t id;
		cost_t cost;
	};

	typedef Vertex::Edge Edge;
	typedef std::pair<cost_t, const Vertex*> PQElement;

	struct greater_pqelement
	{
		bool operator() (const PQElement&, const PQElement&) const;
	};

	typedef std::priority_queue<PQElement, std::vector<PQElement>, greater_pqelement> PriorityQueue;

	//constants
	constexpr static double EARTH_RADIUS_KM = 6372.8;

	//attributes
	std::size_t n_vertices;
	std::size_t n_edges;
	std::map<id_t, Vertex> vertices;

	//private methods
	double degree_to_radian(double) const;
	cost_t heuristic(const Vertex&, const Vertex&) const;

public:
	Graph();
	Graph(const char*);
	void add_vertex(id_t, Location);
	void add_edge(id_t, id_t, cost_t, bool);
	std::size_t vertex_count() const noexcept;
	std::size_t edge_count() const noexcept;
	id_t from_location(Location) const;
	Location location(id_t) const;
	friend std::ostream& operator<<(std::ostream&, const Location&);
	friend std::ostream& operator<< (std::ostream&, const Graph&);
	void output_binary(std::ofstream& out);
	bool dijkstra(id_t, id_t, std::map<id_t, id_t>&) const;
	bool astar(id_t, id_t, std::map<id_t, id_t>&) const;
	std::vector<id_t> reconstruct_path(id_t, id_t, std::map<id_t, id_t>&) const;
};

#endif //GRAPH_HPP