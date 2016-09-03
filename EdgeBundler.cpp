#include "EdgeBundler.h"

EdgeBundler::EdgeBundler(std::vector<EdgeNode> *edges, unsigned int numNeighbors, float curviness)
        : _edges(edges), tree(edges), numNeighbors(numNeighbors), curviness(curviness / 2.0f) {

    double right = 0, top = 0, left = 0, bottom = 0;
    for (auto &e : *edges) {
        Point &p1 = *e.getS();
        Point &p2 = *e.getT();
        left = p1.x < left ? p1.x : left;
        right = p2.x > right ? p2.x : right;
        bottom = p1.y < bottom ? p1.y : bottom;
        bottom = p2.y < bottom ? p2.y : bottom;
        top = p1.y > top ? p1.y : top;
        top = p2.y > top ? p2.y : top;
    }
    width = right - left;
    height = top - bottom;
    center = {width / 2 + left, height / 2 + bottom};
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

void EdgeBundler::findNeighbors(BaseNode *target, int n, std::vector<BaseNode *>neighbors) {
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
                    numBundled++;
                }
            }
        }
        printf("Bundled %d of %ld\n", numBundled, _edges->size());
    } while (numBundled > 0);
}

