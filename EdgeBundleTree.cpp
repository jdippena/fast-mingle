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

BundleNode::BundleNode(EdgeNode *e1, EdgeNode *e2) {
    _ink = e1->calculateBundle(e2, &_s, &_t, &_sCentroid, &_tCentroid);
    e1->_b = this;
    _weight = 2;
    _children.push_back(e1);
    _children.push_back(e2);
}

BundleNode *BundleNode::bundleWith(BaseNode *other) {
    _ink = calculateBundle(other, &_s, &_t, &_sCentroid, &_tCentroid);
    _children.insert(_children.end(), other->getChildren()->begin(), other->getChildren()->end());
    _weight += other->getWeight();
    for (auto c: *other->getChildren()) {
        _children.push_back(c);
        ((EdgeNode *)c)->_b = this;
    }
    return this;
}

BundleNode *EdgeNode::bundleWith(BaseNode *other) {
  if (other->isBundle()) {
      return other->bundleWith(this);   // Merge into other bundle if possible
  } else {
      return new BundleNode(this, (EdgeNode *)other);
  }
}

void BaseNode::ReadEdges(const char *path, std::vector<EdgeNode> &edges) {
      FILE *fp;
      fp = fopen(path, "r");
      assert(fp != NULL);
      int numEdges;
      fscanf(fp, "%i", &numEdges);
      //numEdges /= 10; // HACK FOR TESTING
      std::unordered_map<Point, unsigned int, PointHasher> pointMap;
      Point p1, p2;
      unsigned int idx = 0;
      for (int i = 0; i < numEdges; ++i) {
          fscanf(fp, "%f", &p1.x);
          fscanf(fp, "%f", &p1.y);
          fscanf(fp, "%f", &p2.x);
          fscanf(fp, "%f", &p2.y);
          if (pointMap.find(p1) == pointMap.end()) pointMap[p1] = idx++;
          if (pointMap.find(p2) == pointMap.end()) pointMap[p2] = idx++;
      }

      edges.reserve(numEdges);
      fseek(fp, 0, SEEK_SET);
      fscanf(fp, "%i", &numEdges);
      //numEdges /= 10; // HACK FOR TESTING
      Point *newPoints = (Point*) malloc(sizeof(Point) * idx);
      for (int i = 0; i < numEdges; ++i) {
          fscanf(fp, "%f", &p1.x);
          fscanf(fp, "%f", &p1.y);
          fscanf(fp, "%f", &p2.x);
          fscanf(fp, "%f", &p2.y);

          // Since we treat this as a weighted graph
          /*if (p1.x > p2.x) {
              Point temp = p1;
              p1 = p2;
              p2 = temp;
          }*/

          unsigned int idx1 = pointMap[p1];
          unsigned int idx2 = pointMap[p2];
          newPoints[idx1] = p1;
          newPoints[idx2] = p2;
          Point *s = &newPoints[idx1];
          Point *t = &newPoints[idx2];
          edges.emplace_back(s, t);
      }
      fclose(fp);
}