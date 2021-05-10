#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <queue>
#include "data.hpp"

class Graph : public Data
{
private:
	typedef std::pair<cost_t, Vertex*> PQElement;

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

	inline cost_t heuristic(const Vertex& v1, const Vertex& v2) const 
	{
		const Location& l1 = v1.loc;
		const Location& l2 = v2.loc;

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

public:
	Graph(const char* file) : Data(file)
	{}

	bool dijkstra(id_t start_id, id_t goal_id,
		std::map<id_t, id_t>& came_from)
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

			if (current->id == goal_id) 
			{
				return true;
			}

			for(const Edge& edge : current->edges)
			{
				const cost_t new_cost = cost_so_far[current->id] + edge.cost;

				//if cost does not exist or new_cost is smaller, update cost
				if (cost_so_far.find(edge.destination->id) == cost_so_far.end() ||
					new_cost < cost_so_far[edge.destination->id]) 
				{
					cost_so_far[edge.destination->id] = new_cost;
					came_from[edge.destination->id] = current->id;
					frontier.push(PQElement(new_cost, edge.destination));
				}
			}
		}

		return false;
	}

	bool astar(id_t start_id, id_t goal_id,
		std::map<id_t, id_t>& came_from)
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

			if (current->id == goal_id) 
			{
				return true;
			}

			for(const Edge& edge : current->edges)
			{
				const cost_t new_cost = cost_so_far[current->id] + edge.cost;

				//if cost does not exist or new_cost is smaller, update cost
				if (cost_so_far.find(edge.destination->id) == cost_so_far.end() ||
					new_cost < cost_so_far[edge.destination->id]) 
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

	std::vector<id_t> reconstruct_path(id_t start_id, id_t goal_id,
		std::map<id_t, id_t>& came_from) const
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
};

#endif //GRAPH_HPP