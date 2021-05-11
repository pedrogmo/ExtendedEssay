#ifndef DATA_HPP
#define DATA_HPP

#include <cstdint>
#include <cmath>
#include <map>
#include <list>
#include <limits>
#include <iostream>
#include <fstream>

class Data
{
public:
	typedef double cost_t;
	typedef std::int64_t id_t;

	struct Location
	{
		double lat;
		double lon;
	};

protected:
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

	std::size_t n_vertices;
	std::size_t n_edges;
	std::map<id_t, Vertex> vertices;

public:
	Data()
	: vertices()
	{}

	Data(const char* file)
	{
		std::ifstream in(file, std::ios::binary);

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

	void add_vertex(id_t vertex_id, Location location)
	{
		Vertex v;
		v.id = vertex_id;
		v.loc = location;

		vertices[vertex_id] = v;
		n_vertices++;
	}

	void add_edge(id_t from_id, id_t to_id, cost_t cost, bool one_directional) 
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

	std::size_t vertex_count() const noexcept
	{
		return n_vertices;
	}

	std::size_t edge_count() const noexcept
	{
		return n_edges;
	}

	id_t from_location(Location target)
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

	Location location(id_t vertex_id)
	{
		return vertices.at(vertex_id).loc;
	}

	friend std::ostream& operator<<(std::ostream& out, const Location& l)
	{
		out << l.lat << ", " << l.lon;
		return out;
	}

	friend std::ostream& operator<< (std::ostream& out, const Data& data)
	{
		for(auto it = data.vertices.cbegin(); it != data.vertices.cend(); ++it)
		{
			out << it->first << ": " << it->second.loc << std::endl << "{";
			for(const Edge& e : it->second.edges)
			{
				out << "(" << e.destination->id << ", " << e.cost << "), ";
			}
			out << "}" << std::endl;
		}

		return out;
	}

	void output_binary(std::ofstream& out)
	{
		out.write(reinterpret_cast<const char*>(&n_vertices), sizeof(n_vertices));
		std::map<id_t, Connection> connections;

		for(auto it = vertices.cbegin(); it != vertices.cend(); ++it)
		{
			const Vertex& vertex = it->second;
			const std::size_t n_connections = vertex.edges.size();

			out.write(reinterpret_cast<const char*>(&vertex.id), sizeof(vertex.id));
			out.write(reinterpret_cast<const char*>(&vertex.loc), sizeof(vertex.loc));

			out.write(reinterpret_cast<const char*>(&n_connections), sizeof(n_connections));

			for(const Edge& e : vertex.edges)
			{
				Connection conn = {e.destination->id, e.cost};
				out.write(reinterpret_cast<const char*>(&conn), sizeof(conn));
			}
		}
	}	
};

#endif //DATA_HPP