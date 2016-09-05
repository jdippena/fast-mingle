#include "EdgeBundler.h"

EdgeBundler::EdgeBundler(std::vector<Point> *points, std::vector<EdgeNode> *edges, unsigned int numNeighbors, float curviness)
        : _edges(edges), tree(points, edges), numNeighbors(numNeighbors), curviness(curviness / 2.0f) {
}

void EdgeBundler::rebuildIndex() {
    if (annTree != nullptr) delete(annTree);
    if (annPoints != nullptr)  annDeallocPts(annPoints);
    annPoints = annAllocPts(getNumRootNodes(), 4);
    assert(annPoints != nullptr);
    for (int i = 0; i < getNumRootNodes(); ++i) {
        BaseNode *edge = getRootNode(i);
        ANNpoint p = annPoints[i];
        p[0] = edge->getS()->x;
        p[1] = edge->getS()->y;
        p[2] = edge->getT()->x;
        p[3] = edge->getT()->y;
    }
    annTree = new ANNkd_tree(annPoints, getNumRootNodes(), 4);
    assert(annTree != nullptr);
}

void EdgeBundler::findNeighbors(BaseNode *target, int n, std::vector<BaseNode *>&neighbors) {
    // FIXME: Are these variable length arrays legal?
    ANNidx indices[n];
    ANNdist dists[n];
    ANNcoord queryPoint[4];

    queryPoint[0] = target->getS()->x;
    queryPoint[1] = target->getS()->y;
    queryPoint[2] = target->getT()->x;
    queryPoint[3] = target->getT()->y;
    annTree->annkSearch(queryPoint, n, indices, dists);
    neighbors.resize(n);
    for (int j = 0; j < n; ++j) {
        neighbors[j] = &(*_edges)[indices[j]];
    }
}

void EdgeBundler::doMingle() {
    std::vector<BaseNode *> neighbors;
    int numBundled = 0;
    int iter = 1;
    rebuildIndex();
    do {
        numBundled = 0;
        printf("Iter: %d\n", iter++);
        for (auto &edge : *_edges) {
            if (!edge.hasBundle()) {
                findNeighbors(&edge, numNeighbors, neighbors);
                double maxInkSaved = 0.0;
                BaseNode *bestNeighbor = nullptr;
                for (auto np : neighbors) {
                    double inkSaved = edge.calculateBundleInkSavings(np);
                    if (inkSaved > maxInkSaved) {
                        maxInkSaved = inkSaved;
                        bestNeighbor = np;
                    }
                }
                if (bestNeighbor != nullptr) {
                    edge.bundleWith(bestNeighbor);
                    printf("bundling %u %u and %u %u\n",
                           edge.getS()->id, edge.getT()->id,
                           bestNeighbor->getS()->id, bestNeighbor->getT()->id);
                    numBundled++;
                }
            }
        }
        printf("Bundled %d of %ld\n", numBundled, _edges->size());
    } while (numBundled > 0);
}

