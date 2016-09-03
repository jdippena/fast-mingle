#ifndef MINGLEC_EDGEBUNDLER_H
#define MINGLEC_EDGEBUNDLER_H

#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "ANN/ANN.h"
#include "EdgeBundleTree.h"


class EdgeBundler {
public:
    double width, height;
    Point center;

    EdgeBundler(std::vector<EdgeNode> *edges, unsigned int numNeighbors=10, float curviness=0.5f);
    void doMingle();
    EdgeBundleTree &getTree() { return tree; }

private:
    // Members needed for the ANN search
    ANNpointArray annPoints = nullptr;
    ANNkd_tree *annTree = nullptr;


    unsigned int numNeighbors;
    float curviness;

    EdgeBundleTree tree;

    std::vector<EdgeNode> *_edges;
    std::vector<BaseNode *>rootNodes;

    BaseNode *getRootNode(int i) {
        return rootNodes.empty() ? ((BaseNode *)(&(*_edges)[i])) : rootNodes[i];
    }
    int getNumRootNodes() {
        return (int) (rootNodes.empty() ? _edges->size() : rootNodes.size());
    }

    void readEdgesFromFile(const char *edgeFilename);
    void rebuildIndex();

    void findNeighbors(BaseNode *target, int n, std::vector<BaseNode *> neighbors);
};

#endif //MINGLEC_EDGEBUNDLER_H
