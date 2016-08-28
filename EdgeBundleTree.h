#ifndef MINGLEC_EDGEBUNDLETREE_H
#define MINGLEC_EDGEBUNDLETREE_H

#include <vector>
#include <unordered_set>
#include <cmath>
#include <cassert>
#include "Util.h"




class BaseNode {
public:
    Point* getS() { return _s; };
    Point* getT() { return _t; };
    BundleNode* getBundle() { return _b; }
    virtual Point *getSCentroid() = 0;
    virtual Point *getTCentroid() = 0;
    virtual std::vector<BundleNode *> *getChildren() = 0;
    double getInk() { return _s->sqDist(*_t); }
    void setBundle(BundleNode *b) {
        _b = b;
        for (auto child : *getChildren()) {
          child->setBundle(b);
        }
    }
protected:
    Point *_s = nullptr;
    Point *_t = nullptr;
    BundleNode *_b = nullptr;
};

class BundleNode : public BaseNode {
    Point* getSCentroid() { return &_sCentroid; };
    Point* getTCentroid() { return &_tCentroid; };
    std::vector<BundleNode *> *getChildren() { return &_children; }
private:
    Point _sCentroid;
    Point _tCentroid;
    std::vector<BundleNode *>_children = nullptr;
};


class EdgeNode : public BaseNode {
public:
    Point* getSCentroid() { return _s; };
    Point* getTCentroid() { return _t; };
    std::vector<BundleNode *> *getChildren() { return &NO_POINTS; }
private:
    static std::vector<BundleNode *> NO_POINTS;
};



class EdgeBundleTree {

public:
    struct BundleReturn {
        Point s, t, sCentroid, tCentroid;
        double inkUsed;
    };

    struct Edge {
        std::unordered_set<Point*> *S = new std::unordered_set<Point*>(), *T = new std::unordered_set<Point*>();
        Point *sPoint, *tPoint;
        Point *sCentroid, *tCentroid;
        Edge *bundle;  // highest parent for this edge
        std::vector<Edge*> *children;
        Edge **neighbors;
        double ink;
        int weight;
        bool grouped;
        int id;

        Edge(Point *s, Point *t, Edge *bundle);  // for leaf nodes
        Edge(Edge *child1, Edge *child2, const BundleReturn *data);  // for parent nodes
    };

    Edge *edges;
    unsigned int numEdges;

    EdgeBundleTree(Edge *edges, unsigned int numEdges);

    static double getInkValueFromPoints(Edge& edge0, Edge& edge1, Point& sPoint, Point& tPoint);
    static void testBundle(EdgeBundleTree::BundleReturn *bundleReturn, const Edge &bundle1, const Edge &bundle2);
    static void applyBundle(const BundleReturn& bundleReturn, Edge& edge1, Edge& edge2);
    void coalesceTree();

private:
    double goldenSectionSearch(Edge& node0, Edge& node1);
    static void setBundle(Edge& edge, Edge *bundle);
};


#endif //MINGLEC_EDGEBUNDLETREE_H
