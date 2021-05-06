#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <queue>
#include "data.hpp"

class Graph : public Data
{
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

	constexpr static double EARTH_RADIUS_KM = 6372.8;

	inline double degree_to_radian(double angle) const
	{
		return M_PI * angle / 180.0;
	}

	inline Cost harversine(const Location& l1, const Location& l2) const
	{
		const double lat_rad1 = degree_to_radian(l1.lat);
		const double lat_rad2 = degree_to_radian(l2.lat);
		const double lon_rad1 = degree_to_radian(l1.lon);
		const double lon_rad2 = degree_to_radian(l2.lon);
	 
		const double diff_lat = lat_rad2 - lat_rad1;
		const double diff_lon = lon_rad2 - lon_rad1;
	 
		const double computation = std::asin(std::sqrt(std::sin(diff_lat / 2) * std::sin(diff_lat / 2) + 
			std::cos(lat_rad1) * std::cos(lat_rad2) * std::sin(diff_lon / 2) * std::sin(diff_lon / 2)));

		return 2.0 * EARTH_RADIUS_KM * computation / 2.0;
	}

	inline Cost heuristic(Vertex v1, Vertex v2) const 
	{
		const Location a = locations.at(v1), b = locations.at(v2);
		return harversine(a, b);
	}

public:
	Graph(const char* file) : Data(file)
	{}

	bool dijkstra(Vertex start, Vertex goal,
		std::map<Vertex, Vertex>& came_from)
	{
		std::map<Vertex, Cost> cost_so_far;
		PriorityQueue frontier;

		frontier.push(PQElement(0.0, start));

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

			//get edges from current as pair<it, it> (begin, end)
			const std::pair< std::map<Vertex, Edge>::const_iterator, std::map<Vertex, Edge>::const_iterator > p = edges.equal_range(current);

			for (std::map<Vertex, Edge>::const_iterator it = p.first; it != p.second; ++it) 
			{
				const Edge& edge = it->second;
				const Cost new_cost = cost_so_far[current] + edge.cost;

				//if cost does not exist or new_cost is smaller, update cost
				if (cost_so_far.find(edge.destination) == cost_so_far.end()
					|| new_cost < cost_so_far[edge.destination]) 
				{
					cost_so_far[edge.destination] = new_cost;
					came_from[edge.destination] = current;
					frontier.push(PQElement(new_cost, edge.destination));
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

		frontier.push(PQElement(0.0, start));

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

			//get edges from current as pair<it, it> (begin, end)
			const std::pair< std::map<Vertex, Edge>::const_iterator, std::map<Vertex, Edge>::const_iterator > p = edges.equal_range(current);

			for (std::map<Vertex, Edge>::const_iterator it = p.first; it != p.second; ++it) 
			{
				const Edge& edge = it->second;
				const Cost new_cost = cost_so_far[current] + edge.cost;

				//if cost does not exist or new_cost is smaller, update cost
				if (cost_so_far.find(edge.destination) == cost_so_far.end()
					|| new_cost < cost_so_far[edge.destination]) 
				{
					cost_so_far[edge.destination] = new_cost;
					const Cost priority = new_cost + heuristic(edge.destination, goal);
					came_from[edge.destination] = current;
					frontier.push(PQElement(priority, edge.destination));
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