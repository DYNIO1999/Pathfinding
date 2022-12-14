#ifndef _PATHFINDING_PATHFINDING_H_
#define _PATHFINDING_PATHFINDING_H_
#include <iostream>
#include <vector>
namespace Pathfinding
{

    constexpr size_t GRID_ROW = 60;
    constexpr size_t GRID_COLUMN = 60;
    constexpr size_t NUMBER_OF_AGENTS =GRID_ROW/3;
    
    constexpr int NORMAL_COST =10;
    constexpr int DIAGONAL_COST =14;

    constexpr size_t GRID_SIZE = GRID_ROW * GRID_COLUMN;

    struct Node
    {
        int i;
        int j;
        int previousNode; //Id of previous node to reconstruct path
        float Fcost; // GCost + HCost
        float Hcost; // Distance to End Node
        float Gcost; // Distance to Start Node
        int passable; //Passable or Impassable
        int ID;
        int neighbours[8];
    };

    struct GridData{
        int start;
        int end;
        Node nodes[GRID_SIZE];
    };

    struct Path
    {
        int pathList[GRID_SIZE];
    };
    class AStar{
    public:
    static int CalculateDistance(int ax, int ay, int bx, int by);
    static std::vector<int> FindPath(GridData& grid);
    
    static int FindIndex(int i, int j);
    };
    
} 
#endif