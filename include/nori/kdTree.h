//
// Created by 打工人 on 2022/5/10.
//

#ifndef NORI_KDTREE_H
#define NORI_KDTREE_H


#include <utility>
#include <queue>
#include "nori/common.h"
#include "emitter.h"

NORI_NAMESPACE_BEGIN


    static Vector3f max(Vector3f &a, Vector3f &b) {
        return {std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z())};
    }

    static Vector3f min(Vector3f &a, Vector3f &b) {
        return {std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z())};
    }


    struct KdNode {
        int axis{}; // 0 1 2
        Photon *data{nullptr};
        BoundingBox3d bbox;
        KdNode *left{nullptr};
        KdNode *right{nullptr};
    };

    class KdTree {

    public:
        KdTree() {}

//    typedef  std::pair<Vector3f , *> Record;
        void build(std::vector<Photon> &elements);

        std::vector<Photon *> kNN(const Vector3f &pos, int k) const;

    private:
        KdNode *root;

    };


    void KdTree::build(std::vector<Photon> &photons) {


        std::function<KdNode *(std::vector<Photon> &, int, int,int)> solve = [&](std::vector<Photon> &elements, int l,
                                                                             int r,int depth) -> KdNode * {
            if (r == l)
                return nullptr;
            Vector3f mx = elements[0].position;
            Vector3f mn = elements[0].position;

//            for (int i = l; i < r; i++) {
//                Photon &p = elements[i];
//                mx = max(p.position, mx);
//                mn = min(p.position, mn);
//            }

            KdNode *node = new KdNode;


            auto maxAxis = [&](Vector3f &vec) -> int {
                if (vec.x() > vec.y() && vec.x() > vec.z())
                    return 0;
                if (vec.y() > vec.z())
                    return 1;
                return 2;
            };
            Vector3f temp = mx - mn;
//            node->axis = maxAxis(temp);
            node->axis=depth%3;

            auto cmp = [&](const Photon &a, Photon &b) {
                return a.position[node->axis] < b.position[node->axis];
            };

            std::sort(elements.begin() + l, elements.begin() + r, cmp);

            BoundingBox3d box;
            for (int i=l;i<r;i++) {
                Photon photon=photons[i];
                Vector3f ppos = photon.position;
                box.expandBy(Point3d(ppos.x(), ppos.y(), ppos.z()));
            }
            node->bbox = box;


            int mid = (l + r) / 2;

            node->data = &elements[mid];
            node->left = solve(elements, l, mid,depth+1);
            node->right = solve(elements, mid + 1, r,depth+1);

            return node;
        };
        root = solve(photons, 0, photons.size(),0);

    }

    std::vector<Photon *> KdTree::kNN(const Vector3f &pos, int k) const {

        auto cmp = [&](Photon *a, Photon *b) {
            float dist_a = (a->position - pos).squaredNorm();
            float dist_b = (b->position - pos).squaredNorm();
            return dist_a < dist_b;
        };


        std::function<float(const Vector3f &)> eval = [&](const Vector3f &point) {
            return (pos - point).squaredNorm();
        };

        std::priority_queue<Photon *, std::vector<Photon *>, decltype(cmp)> photon_queue(cmp);


        std::function<float()> currentBound = [&]() {
            if (photon_queue.size() < k) {
                return 1e15f;
            }

            auto ans = eval(photon_queue.top()->position);
            return ans;
        };


        std::function<void(Photon *)> pushPhoton = [&](Photon *photon) {
            if (eval(photon->position) < currentBound())
                photon_queue.push(photon);
            if (photon_queue.size() > k)
                photon_queue.pop();
        };


        std::function<void(const KdNode *)> solve = [&](const KdNode *node) {
            if (!node) {
                return;
            }

            BoundingBox3d bbox = node->bbox;


            if (!bbox.contains(Point3d(pos.x(), pos.y(), pos.z()))) {
                auto mx = std::min(abs(bbox.min.x() - pos.x()), abs(bbox.max.x() - pos.x()));
                if (bbox.min.x() < pos.x() && bbox.max.x() > pos.x()) mx = 0;
                auto my = std::min(abs(bbox.min.y() - pos.y()), abs(bbox.max.y() - pos.y()));
                if (bbox.min.y() < pos.y() && bbox.max.y() > pos.y()) my = 0;
                auto mz = std::min(abs(bbox.min.z() - pos.z()), abs(bbox.max.z() - pos.z()));
                if (bbox.min.z() < pos.z() && bbox.max.z() > pos.x()) mz = 0;
                Vector3f d(mx, my, mz);

                if (d.squaredNorm() > currentBound())  //没有找到更近点的可能 就返回
                    return;
            }



            pushPhoton(node->data);

            if (pos[node->axis] < root->data->position[node->axis]) {
                solve(node->left);
                if(currentBound()> pow(pos[node->axis]-node->data->position[node->axis],2))
                solve(node->right);
            } else {
                solve(node->right);
                if(currentBound()> pow(pos[node->axis]-node->data->position[node->axis],2))
                solve(node->left);
            }

        };

        solve(root);

        std::vector<Photon *> ans;

        while (!photon_queue.empty()) {
            ans.push_back(photon_queue.top());
            photon_queue.pop();
        }

        return ans;

    }


NORI_NAMESPACE_END


#endif //NORI_KDTREE_H
