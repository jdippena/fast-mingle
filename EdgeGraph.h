//
// Created by Shilad.Sen on 9/3/16.
//

#ifndef MINGLEC_EDGEGRAPH_H
#define MINGLEC_EDGEGRAPH_H


#include "Util.h"

#define ID_NONE    0

typedef uint32_t GraphNodeId;


struct GraphNode {
    GraphNodeId _id;                // same as index in vector below
    PointId _s;                     // point ids
    PointId _t;                     // point ids
    GraphNodeId _parent = ID_NONE;  // could be ID_NONE
    PointId *_children = nullptr;   // null for leafs, terminated by ID_NONE otherwise
    int weight = 1;

    GraphNode(PointId s, PointId t, GraphNodeId id) : _s(s), _t(t), _id(id) {}
};

class EdgeGraph {
public:
    EdgeGraph() {}

static EdgeGraph *Read(char *nodePath, char *edgePath);


private:
    GraphNodeId  _maxId = 0;
    std::vector<Point>_points;     // indexed by point id;
    std::vector<GraphNode>_nodes;
};

#endif //MINGLEC_EDGEGRAPH_H
