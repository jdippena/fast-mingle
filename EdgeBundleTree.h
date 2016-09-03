#ifndef MINGLEC_EDGEBUNDLETREE_H
#define MINGLEC_EDGEBUNDLETREE_H

#include <vector>
#include <unordered_set>
#include <cmath>
#include <cassert>
#include <list>
#include <deque>
#include "Util.h"

class BaseNode;
class BundleNode;
class EdgeNode;


class BaseNode {
public:
    virtual Point *getS() = 0;
    virtual Point *getT() = 0;
    virtual Point *getSCentroid() = 0;
    virtual Point *getTCentroid() = 0;
    virtual int getWeight() = 0;
    virtual BundleNode* bundleWith(BaseNode *Other) = 0;
    virtual std::vector<BaseNode *> *getChildren() = 0;
    virtual BundleNode* getBundle() = 0;
    virtual bool hasBundle() {  return getBundle() != nullptr; }
    virtual bool isBundle() = 0;
    virtual double getInk() = 0;

    double calculateBundleInkSavings(BaseNode *other);

    static void ReadEdges(const char *path, std::vector<EdgeNode> &edges);
protected:
    double calculateBundle(BaseNode *other,
                           Point *s, Point *t,
                           Point *sCentroid, Point *tCentroid);
};

class BundleNode : public BaseNode {
public:
    BundleNode(EdgeNode *e1, EdgeNode *e2);

    BundleNode* getBundle() { return this; }
    Point* getS() { return &_s; };
    Point* getT() { return &_t; };
    Point* getSCentroid() { return &_sCentroid; };
    Point* getTCentroid() { return &_tCentroid; };
    std::vector<BaseNode *> *getChildren() { return &_children; }
    BundleNode* bundleWith(BaseNode *Other);
    int getWeight() { return _weight; }
    bool isBundle() { return true; }
    double getInk() { return _ink; }

private:
    Point _s;
    Point _t;
    Point _sCentroid;
    Point _tCentroid;
    std::vector<BaseNode *> _children;
    int _weight = 0;
    double _ink;
    friend class EdgeNode;
};

static std::vector<BaseNode *> NO_POINTS;

class EdgeNode : public BaseNode {
public:
    EdgeNode(Point *s, Point *t) : _s(s), _t(t) {}
    Point* getS() { return _s; };
    Point* getT() { return _t; };
    Point* getSCentroid() { return _s; };
    Point* getTCentroid() { return _t; };
    int getWeight() { return 1; }
    std::vector<BaseNode *> *getChildren() { return &NO_POINTS; }
    BundleNode* bundleWith(BaseNode *Other);
    bool isBundle() { return false; }
    bool hasBundle() { return _b != nullptr; }
    BundleNode* getBundle() { return _b; }
    double getInk() { return _s->sqDist(*_t); }

private:
    BundleNode *_b = nullptr;   // Top level bundle
    Point *_s = nullptr;
    Point *_t = nullptr;
    friend class BundleNode;
};

/**
 * Fixme: Convert to range-based iterator
 * Usage: call next() over and over until it returns nullptr.
 */
class EdgeBundleIterator {
public:
    EdgeBundleIterator(std::vector<EdgeNode> *edges) : _edges(edges) {}

    BaseNode *next() {
        while (true) {
            if (!_frontier.empty()) {
                // Something on the frontier.
                // Pop it, push its children (if any), return it.
                BaseNode *n = _frontier.back();
                _frontier.pop_back();
                for (auto child : *n->getChildren()) {
                    _frontier.push_back(child);
                }
                return n;
            } else if (_edgeIndex < _edges->size()) {
                // More raw edges remain. Check the next...
                BaseNode *n = &(*_edges)[_edgeIndex++];

                BaseNode *b = n->getBundle();

                // If it's an orphaned edge, visit it
                if (b == nullptr) {
                  return b;
                }

                // If there's a root bundle, check to see if it's
                // already visited or not. Add it to the frontier
                // if not and cycle around to the next iteration.
                if (_visitedBundles.find(b) != _visitedBundles.end()) {
                    _visitedBundles.insert(b);
                    _frontier.push_back(b);
                }
            } else {
                return nullptr;
            }
        }
    }
private:
    int _edgeIndex = 0;
    std::vector<EdgeNode> *_edges;        // Raw, unbundled edges
    std::unordered_set<BaseNode *> _visitedBundles;
    std::deque<BaseNode *> _frontier;

};

class EdgeBundleTree {
public:
    EdgeBundleTree(std::vector<EdgeNode> *edges) : _edges(edges) {}

    std::vector<EdgeNode> *_edges;        // Raw, unbundled edges

    EdgeBundleIterator *iterator() {
        return new EdgeBundleIterator(_edges);
    }

    void getRootNodes(std::vector<BaseNode *> &result) {
        std::unordered_set<BaseNode *> visitedBundles;
        result.clear();
        for (EdgeNode &e : *_edges) {
            if (!e.hasBundle()) {
                result.push_back(&e);
            } else {
                BaseNode *b = e.getBundle();
                if (visitedBundles.find(b) == visitedBundles.end()) {
                    visitedBundles.insert(b);
                    result.push_back(b);
                }
            }
        }
    }
};






#endif //MINGLEC_EDGEBUNDLETREE_H
