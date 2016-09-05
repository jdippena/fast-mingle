#include <unordered_map>
#include "EdgeBundleTree.h"

double BaseNode::calculateBundleInkSavings(BaseNode *other) {
  double newInk = calculateBundle(other, nullptr, nullptr, nullptr, nullptr);
  return getInk() + other->getInk() - newInk;
}

double BaseNode::calculateBundle(BaseNode *other,
                                 Point *sp, Point *tp,
                                 Point *sCentroidp, Point *tCentroidp) {

    int combinedWeight = getWeight() + other->getWeight();
    Point sCentroid = {
            (getSCentroid()->x * getWeight() + other->getSCentroid()->x * getWeight()) / combinedWeight,
            (getSCentroid()->y * getWeight() + other->getSCentroid()->y * getWeight()) / combinedWeight
    };
    Point tCentroid = {
            (getTCentroid()->x * getWeight() + other->getTCentroid()->x * getWeight()) / combinedWeight,
            (getTCentroid()->y * getWeight() + other->getTCentroid()->y * getWeight()) / combinedWeight
    };

    Point u = tCentroid - sCentroid;
    u = u / u.norm();
    double dist, distSum = 0, maxDist = 0;
    int k = 0;
    for (auto n : {this, other}) {
      for (auto child : *n->getChildren()) {
          Point v = *child->getS() - sCentroid;
          dist = u * v;
          maxDist = maxDist < dist ? dist : maxDist;
          distSum += dist;
          k += 1;
      }
    }
     for (auto n : {this, other}) {
      for (auto child : *n->getChildren()) {
          Point v = *child->getT() - tCentroid;
          dist = -(u * v);
          maxDist = maxDist < dist ? dist : maxDist;
          distSum += dist;
          k += 1;
      }
    }

    Point delta = tCentroid - sCentroid;
    double d = delta.norm();
    double x = ((distSum + 2 * d) / (k + 4) / d);
    Point sPoint = lerp(sCentroid, tCentroid, x);
    Point tPoint = lerp(sCentroid, tCentroid, 1 - x);
    delta = tPoint - sPoint;
    double inkValueCombined = delta.norm();
    for (auto n : {this, other}) {
        for (auto child : *n->getChildren()) {
            delta = *child->getS() - sPoint;
            inkValueCombined += delta.norm();
        }
    }
    for (auto n : {this, other}) {
        for (auto child : *n->getChildren()) {
            delta = *child->getT() - tPoint;
            inkValueCombined += delta.norm();
        }
    }
    if (sp != nullptr) *sp = sPoint;
    if (tp != nullptr) *tp = tPoint;
    if (sCentroidp != nullptr) *sCentroidp = sCentroid;
    if (tCentroidp != nullptr) *tCentroidp = tCentroid;
    return inkValueCombined;
}

BundleNode::BundleNode(BaseNode *e1, BaseNode *e2) {
    _ink = e1->calculateBundle(e2, &_s, &_t, &_sCentroid, &_tCentroid);
    e1->_b = this;
    e2->_b = this;
    _weight = 2;
    _children.push_back(e1);
    _children.push_back(e2);
}

void BundleNode::annexBundle(BaseNode *other) {
    assert(other->isBundle());
    _ink = calculateBundle(other, &_s, &_t, &_sCentroid, &_tCentroid);
    _children.insert(_children.end(), other->getChildren()->begin(), other->getChildren()->end());
    _weight += other->getWeight();
    for (auto c: *other->getChildren()) {
        _children.push_back(c);
        ((EdgeNode *)c)->_b = this;
    }
}

void BundleNode::addChild(BaseNode *other) {
    assert(!other->hasBundle());
    _ink = calculateBundle(other, &_s, &_t, &_sCentroid, &_tCentroid);
    _children.push_back(other);
    _weight += other->getWeight();
    other->_b = this;
}

void BaseNode::bundleWith(BaseNode *other) {
    if (!this->hasBundle() && !other->hasBundle()) {
        // If neither are bundled, bundle them with a new bundle
        new BundleNode(this, (EdgeNode *)other);
    } else if (this->getBundle() == other->getBundle()) {
        // If they have the same bundle we are done!
        return;
    } else if (this->hasBundle() && other->hasBundle()) {
        // if they both have a bundle, merge into one and destroy other
        BaseNode *n = other->getBundle();
        this->getBundle()->annexBundle(other->getBundle());
        delete n;
    } else if (this->hasBundle()) {
        // If this one has a bundle, bundle with this one
        this->getBundle()->addChild(other);
    } else {
        // Bundle with other one.
        assert(other->hasBundle());
        other->getBundle()->addChild(this);
    }
}

void BaseNode::ReadEdges(const char *path,
                         std::vector<Point> &nodes,
                         std::vector<EdgeNode> &edges) {

    edges.clear();
    nodes.clear();
    nodes.push_back(Point());   // Node 0 is unused; a place holder

    FILE *fp = fopen(path, "r");
    assert(fp != NULL);

    int numEdges;
    std::unordered_map<Point, PointId, PointHasher> pointMap;
    fscanf(fp, "%i", &numEdges);
    //numEdges /= 10; // HACK FOR TESTING
    edges.reserve(numEdges);

    Point p1, p2;
    // Take a pass through first to find all the points so that
    // The memory addresses are fixed and we can use pointers.
    for (int i = 0; i < numEdges; ++i) {
        fscanf(fp, "%f", &p1.x);
        fscanf(fp, "%f", &p1.y);
        fscanf(fp, "%f", &p2.x);
        fscanf(fp, "%f", &p2.y);

        // Since we treat this as a weighted graph
        if (p1.x > p2.x) {
            Point temp = p1;
            p1 = p2;
            p2 = temp;
        }

        //
        PointId idx1 = pointMap[p1];
        PointId idx2 = pointMap[p2];

        if (idx1 == POINT_ID_NONE) {
            nodes.emplace_back(p1.x, p1.y);
            idx1 = nodes.back().id;
            pointMap[p1] = idx1;
        }
        if (idx2 == POINT_ID_NONE) {
            nodes.emplace_back(p2.x, p2.y);
            idx2 = nodes.back().id;
            pointMap[p2] = idx2;
        }
    }

    rewind(fp);
    fscanf(fp, "%i", &numEdges);
    for (int i = 0; i < numEdges; ++i) {
        fscanf(fp, "%f", &p1.x);
        fscanf(fp, "%f", &p1.y);
        fscanf(fp, "%f", &p2.x);
        fscanf(fp, "%f", &p2.y);

        // Since we treat this as a weighted graph
        if (p1.x > p2.x) {
            Point temp = p1;
            p1 = p2;
            p2 = temp;
        }

        PointId idx1 = pointMap[p1];
        PointId idx2 = pointMap[p2];
        assert(idx1 != POINT_ID_NONE);
        assert(idx2 != POINT_ID_NONE);

        Point *s = &nodes[idx1];
        Point *t = &nodes[idx2];
        edges.emplace_back(s, t);
        EdgeNode &e = edges.back();
    }
    fclose(fp);
}