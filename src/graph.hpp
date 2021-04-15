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

	inline double heuristic(Vertex v1, Vertex v2) const
	{
		Location a = locations.at(v1), b = locations.at(v2);
		return std::abs(a.lat - b.lat) + std::abs(a.lon - b.lon);
	}

public:
	Graph(const char* file) : Data(file)
	{}

	bool dijkstra(Vertex start, Vertex goal,
		std::map<Vertex, Vertex>& came_from)
	{
		std::map<Vertex, Cost> cost_so_far;
		PriorityQueue frontier;

		frontier.push({0.0, start});

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
					frontier.push({new_cost, edge.destination});
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

		frontier.push({0.0, start});

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
					frontier.push({priority, edge.destination});
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