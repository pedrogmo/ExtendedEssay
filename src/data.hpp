#ifndef DATA_HPP
#define DATA_HPP

#include <cstdint>
#include <cmath>
#include <map>
#include <unordered_map>
#include <limits>
#include <iostream>
#include <fstream>

class Data
{
public:
	typedef std::int64_t Vertex;
	typedef double Cost;

	struct Location
	{
		double lat;
		double lon;
	};
	struct Edge
	{
		Vertex destination;
		Cost cost;
	};

protected:

	std::unordered_map<Vertex, Location> locations;
	std::multimap<Vertex, Edge> edges;

public:
	Data()
	: locations(), edges()
	{}

	Data(const char* file)
	{
		std::ifstream in(file);

		std::uint32_t n_vertices;
		in.read(reinterpret_cast<char*>(&n_vertices), sizeof(n_vertices));

		for(; n_vertices > 0; --n_vertices)
		{
			Vertex n;
			Location l;

			in.read(reinterpret_cast<char*>(&n), sizeof(n));
			in.read(reinterpret_cast<char*>(&l.lat), sizeof(l.lat));
			in.read(reinterpret_cast<char*>(&l.lon), sizeof(l.lon));

			locations[n] = l;
		}

		std::uint32_t n_edges;
		in.read(reinterpret_cast<char*>(&n_edges), sizeof(n_edges));

		for(; n_edges > 0; --n_edges)
		{
			Vertex from;
			Edge e;

			in.read(reinterpret_cast<char*>(&from), sizeof(from));
			in.read(reinterpret_cast<char*>(&e.destination), sizeof(e.destination));
			in.read(reinterpret_cast<char*>(&e.cost), sizeof(e.cost));

			edges.insert({from, e});
		}

		in.close();
	}

	void add_location(Vertex v, Location loc)
	{
		locations[v] = loc;
	}

	void add_edge(Vertex from, Edge e)
	{
		edges.insert({from, e});
	}

	std::uint32_t vertex_count() const
	{
		return locations.size();
	}

	std::uint32_t edge_count() const
	{
		return edges.size();
	}

	Vertex from_location(Location target)
	{
		Vertex closest_match;
		double minimum_difference = std::numeric_limits<double>::max();

		for(auto it = locations.cbegin(); it != locations.cend(); ++it)
		{
			Vertex n = it->first;
			Location l = it->second;
			const double diff = std::abs(target.lat - l.lat) + std::abs(target.lon - l.lon);

			if (diff < minimum_difference)
			{
				closest_match = n;
				minimum_difference = diff;
			}
		}

		return closest_match;
	}

	friend std::ostream& operator<< (std::ostream& out, const Data& data)
	{
		for(auto it = data.edges.cbegin(); it != data.edges.cend(); ++it)
		{
			out << it->first << " -> " << it->second.destination << ": " << it->second.cost << std::endl;
		}

		return out;
	}

	void output_binary(std::ofstream& out)
	{
		const std::uint32_t n_vertices = vertex_count();
		out.write(reinterpret_cast<const char*>(&n_vertices), sizeof(n_vertices));

		for(auto it = locations.cbegin(); it != locations.cend(); ++it)
		{
			Vertex n = it->first;
			Location l = it->second;

			out.write(reinterpret_cast<const char*>(&n), sizeof(n));
			out.write(reinterpret_cast<const char*>(&l.lat), sizeof(l.lat));
			out.write(reinterpret_cast<const char*>(&l.lon), sizeof(l.lon));
		}

		const std::uint32_t n_edges = edge_count();
		out.write(reinterpret_cast<const char*>(&n_edges), sizeof(n_edges));

		for(auto it = edges.cbegin(); it != edges.cend(); ++it)
		{
			Vertex from = it->first;
			Edge e = it->second;

			out.write(reinterpret_cast<const char*>(&from), sizeof(from));
			out.write(reinterpret_cast<const char*>(&e.destination), sizeof(e.destination));
			out.write(reinterpret_cast<const char*>(&e.cost), sizeof(e.cost));
		}
	}
};

#endif //DATA_HPP