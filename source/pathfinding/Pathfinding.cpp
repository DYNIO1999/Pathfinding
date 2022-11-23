#include "Pathfinding.h"
#include "../core/Logger.h"
namespace Pathfinding{

    int AStar::CalculateDistance(int ax, int ay, int bx, int by)
    {
        int dx = std::abs(ax - bx);
        int dy = std::abs(ay - by);
        if (dx > dy)
            return 14 * dy + 10 * (dx - dy);
        return 14 * dx + 10 * (dy - dx);
    }

    int AStar::FindIndex(int i, int j){
        return (GRID_ROW * i) + j;
    }


    std::vector<int> AStar::FindPath(GridData& grid){
        std::vector<int> path;

        std::vector<int> toSearchNodes = {grid.start};
        std::vector<int> processedNodes;

        while (!toSearchNodes.empty())
        {
            int currentNode = toSearchNodes[0];
            for (size_t i = 0; i < toSearchNodes.size(); i++)
            {
                if ((grid.nodes[toSearchNodes[i]].Fcost < grid.nodes[currentNode].Fcost) && (grid.nodes[toSearchNodes[i]].Hcost < grid.nodes[currentNode].Hcost))
                {
                    currentNode = toSearchNodes[i];
                }
            }

            auto toBeRemoved = std::find(toSearchNodes.begin(), toSearchNodes.end(), currentNode);
            toSearchNodes.erase(toBeRemoved);
            processedNodes.push_back(currentNode);

            if (currentNode == grid.end)
            {
                path.clear();
                int currentIDBack = grid.end;

                while ((currentIDBack != -1) && (currentIDBack != grid.start))
                {
                    path.push_back(currentIDBack);
                    currentIDBack = grid.nodes[currentIDBack].previousNode;
                }
                path.push_back(grid.start);
                //path.erase(path.begin());
                break;
            }

            for (int i = 0; i < 8; i++)
            {
                int neighbour = grid.nodes[currentNode].neighbours[i];

                bool foundInProcessed = false;
                auto checkProcessed = std::find(processedNodes.begin(), processedNodes.end(), neighbour);
                if (checkProcessed != processedNodes.end())
                {
                    foundInProcessed = true;
                }

                if ((neighbour != -1) && (!foundInProcessed) && grid.nodes[neighbour].passable)
                {
                    
                    
                    int costToNeighbour = grid.nodes[currentNode].Gcost + CalculateDistance(grid.nodes[currentNode].i, grid.nodes[currentNode].j, grid.nodes[neighbour].i, grid.nodes[neighbour].j);
                    bool found = false;
                    auto check = std::find(toSearchNodes.begin(), toSearchNodes.end(), neighbour);
                    if (check != toSearchNodes.end())
                    {
                        found = true;
                    }

                    if ((costToNeighbour < grid.nodes[neighbour].Gcost) || (!found))
                    {
                        grid.nodes[neighbour].Gcost = costToNeighbour;
                        grid.nodes[neighbour].previousNode = currentNode;
                        grid.nodes[neighbour].Hcost = CalculateDistance(grid.nodes[neighbour].i, grid.nodes[neighbour].j, grid.nodes[grid.end].i, grid.nodes[grid.end].j);
                        grid.nodes[neighbour].Fcost = grid.nodes[neighbour].Gcost + grid.nodes[neighbour].Hcost;

                        if (!found)
                        {
                            toSearchNodes.push_back(neighbour);
                        }
                    }
                }
            }
        }
   

        std::reverse(path.begin(), path.end());
        for (int i = 0; i < GRID_COLUMN; i++)
        {
            for (int j = 0; j < GRID_COLUMN; j++)
            {
                grid.nodes[FindIndex(i, j)].Fcost = 0.0f;
                grid.nodes[FindIndex(i, j)].Gcost = 0.0f;
                grid.nodes[FindIndex(i, j)].Hcost = 0.0f;
                grid.nodes[FindIndex(i, j)].previousNode = -1;
            }
        }

        //done = true;
    

        return path;
    }

}