/*
 *  Copyright 2023 Tucker Irwin
 */

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include <iostream>
#include <algorithm>
#include <stack>
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GPath.h"
#include "ExtraShaders.h"

class MyCanvas : public GCanvas {
public:
    struct Edge {
        float m;      // Slope
        float b;      // Y-intercept
        float top;    // Top coordinate
        float bottom; // Bottom coordinate

        Edge(float _m, float _b, float _top, float _bottom)
            : m(_m), b(_b), top(_top), bottom(_bottom) {}
    };

    struct pEdge {
        float m;      // Slope
        float b;      // Y-intercept
        int top;    // Top coordinate
        int bottom; // Bottom coordinate
        int dir;    // winding

        bool edgeCheck() {
            return top < bottom;
        }
        
        pEdge(GPoint p0, GPoint p1, int height) {
            float run = p1.x - p0.x;
            float rise = p1.y - p0.y;
            m = run / rise;
            b = p0.x - m * p0.y;
            top = std::max(0, std::min(GRoundToInt(p0.y), GRoundToInt(p1.y)));
            bottom = std::min(height, std::max(GRoundToInt(p0.y), GRoundToInt(p1.y)));
            dir = p0.y < p1.y ? -1 : 1;
        }
    };

    struct Vertex {
        GPoint p;   // point
        GColor c;   // color
        GPoint t;   // texture
        bool hasColor = false;
        bool hasTexture = false;
        
        Vertex(GPoint _p) : p(_p) {}
        Vertex(GPoint _p, GColor _c, GPoint _t) 
            : p(_p), c(_c), t(_t), hasColor(true), hasTexture(false) {}
        
        void setColor(GColor _c) {
            c = _c;
            hasColor = true;
        }
        void setTexture(GPoint _t) {
            t = _t;
            hasTexture = true;
        }
    };

    MyCanvas(const GBitmap& device) : fDevice(device) {
        ctmStack.push(*(new GMatrix()));
    }

    GPixel convert(GColor color){
        float ra = color.r * color.a;
        float ga = color.g * color.a;
        float ba = color.b * color.a;
        unsigned R = GRoundToInt(ra * 255);
        unsigned G = GRoundToInt(ga * 255);
        unsigned B = GRoundToInt(ba * 255);
        unsigned A = GRoundToInt(color.a * 255);
        return GPixel_PackARGB(A, R, G, B);
    }

    void save() override {
        GMatrix saved = ctmStack.top();
        GMatrix copy(
            saved[0], saved[1], saved[2],
            saved[3], saved[4], saved[5]
        );
        ctmStack.push(copy);
    }

    void restore() override {
        if (!ctmStack.empty()){
            ctmStack.pop();
        }
    }

    void concat(const GMatrix& matrix) override {
        ctmStack.top() = ctmStack.top().Concat(ctmStack.top(), matrix);
        //ctmStack.top() = ctmStack.top() * matrix;
    }

    GPixel kClear(GPixel src, GPixel dst){
        return GPixel_PackARGB(1, 0, 0, 0);
    }

    GPixel kSrc(GPixel src, GPixel dst){
        return src;
    }

    GPixel kDst(GPixel src, GPixel dst){
        return dst;
    }
    
    GPixel kSrcOver(GPixel src, GPixel dst){
        float dCoeff = (255.0f - GPixel_GetA(src))/255.0f;
        unsigned rr = GRoundToInt(GPixel_GetR(src) + dCoeff * GPixel_GetR(dst));
        unsigned gr = GRoundToInt(GPixel_GetG(src) + dCoeff * GPixel_GetG(dst));
        unsigned br = GRoundToInt(GPixel_GetB(src) + dCoeff * GPixel_GetB(dst));
        unsigned ar = GRoundToInt(GPixel_GetA(src) + dCoeff * GPixel_GetA(dst));
        return GPixel_PackARGB(ar, rr, gr, br);
    }

    GPixel kDstOver(GPixel src, GPixel dst){
        float dCoeff = (255.0f - GPixel_GetA(dst))/255.0f;
        unsigned rr = GRoundToInt(GPixel_GetR(dst) + dCoeff * GPixel_GetR(src));
        unsigned gr = GRoundToInt(GPixel_GetG(dst) + dCoeff * GPixel_GetG(src));
        unsigned br = GRoundToInt(GPixel_GetB(dst) + dCoeff * GPixel_GetB(src));
        unsigned ar = GRoundToInt(GPixel_GetA(dst) + dCoeff * GPixel_GetA(src));
        return GPixel_PackARGB(ar, rr, gr, br);
    }

    GPixel kSrcIn(GPixel src, GPixel dst){
        float dCoeff = GPixel_GetA(dst)/255.0f;
        unsigned rr = GRoundToInt(dCoeff * GPixel_GetR(src));
        unsigned gr = GRoundToInt(dCoeff * GPixel_GetG(src));
        unsigned br = GRoundToInt(dCoeff * GPixel_GetB(src));
        unsigned ar = GRoundToInt(dCoeff * GPixel_GetA(src));
        return GPixel_PackARGB(ar, rr, gr, br);
    }

    GPixel kDstIn(GPixel src, GPixel dst){
        float sCoeff = GPixel_GetA(src)/255.0f;
        unsigned rr = GRoundToInt(sCoeff * GPixel_GetR(dst));
        unsigned gr = GRoundToInt(sCoeff * GPixel_GetG(dst));
        unsigned br = GRoundToInt(sCoeff * GPixel_GetB(dst));
        unsigned ar = GRoundToInt(sCoeff * GPixel_GetA(dst));
        return GPixel_PackARGB(ar, rr, gr, br);
    }

    GPixel kSrcOut(GPixel src, GPixel dst){
        float dCoeff = (255.0f - GPixel_GetA(dst))/255.0f;
        unsigned rr = GRoundToInt(dCoeff * GPixel_GetR(src));
        unsigned gr = GRoundToInt(dCoeff * GPixel_GetG(src));
        unsigned br = GRoundToInt(dCoeff * GPixel_GetB(src));
        unsigned ar = GRoundToInt(dCoeff * GPixel_GetA(src));
        return GPixel_PackARGB(ar, rr, gr, br);
    }

    GPixel kDstOut(GPixel src, GPixel dst){
        float dCoeff = (255.0f - GPixel_GetA(src))/255.0f;
        unsigned rr = GRoundToInt(dCoeff * GPixel_GetR(dst));
        unsigned gr = GRoundToInt(dCoeff * GPixel_GetG(dst));
        unsigned br = GRoundToInt(dCoeff * GPixel_GetB(dst));
        unsigned ar = GRoundToInt(dCoeff * GPixel_GetA(dst));
        return GPixel_PackARGB(ar, rr, gr, br);
    }

    GPixel kSrcATop(GPixel src, GPixel dst){
        GPixel part1 = kSrcIn(src, dst);
        GPixel part2 = kDstOut(src, dst);
        GPixel result = GPixel_PackARGB(GPixel_GetA(part1) + GPixel_GetA(part2), 
                GPixel_GetR(part1) + GPixel_GetR(part2), GPixel_GetG(part1) + GPixel_GetG(part2),
                GPixel_GetB(part1) + GPixel_GetB(part2));
        return result;
    }

    GPixel kDstATop(GPixel src, GPixel dst){
        GPixel part1 = kDstIn(src, dst);
        GPixel part2 = kSrcOut(src, dst);
        GPixel result = GPixel_PackARGB(GPixel_GetA(part1) + GPixel_GetA(part2), 
                GPixel_GetR(part1) + GPixel_GetR(part2), GPixel_GetG(part1) + GPixel_GetG(part2),
                GPixel_GetB(part1) + GPixel_GetB(part2));
        return result;
    }

    GPixel kXor(GPixel src, GPixel dst){
        GPixel part1 = kDstOut(src, dst);
        GPixel part2 = kSrcOut(src, dst);
        GPixel result = GPixel_PackARGB(GPixel_GetA(part1) + GPixel_GetA(part2), 
                GPixel_GetR(part1) + GPixel_GetR(part2), GPixel_GetG(part1) + GPixel_GetG(part2),
                GPixel_GetB(part1) + GPixel_GetB(part2));
        return result;
    }

    void clear(const GColor& color) override {
        GPixel pColor = convert(color);
        for (int y = 0; y < fDevice.height(); y++) {
            for (int x = 0; x < fDevice.width(); x++) {
                *fDevice.getAddr(x, y) = pColor;
            }
        }
    }
    
    void drawRect(const GRect& rect, const GPaint& paint) override {
        GMatrix ctm = ctmStack.top();
        if (ctm[0] != 1 || ctm[1] != 0 || ctm[2] != 0 || ctm[3] != 0 || ctm[4] != 1 || ctm[5] != 0) {
            GPoint points[4];
            points[0].y = rect.top; points[0].x = rect.left;
            points[3].y = rect.top; points[3].x = rect.right;
            points[1].y = rect.bottom; points[1].x = rect.left;
            points[2].y = rect.bottom; points[2].x = rect.right;
            drawConvexPolygon(points, 4, paint);
            return;
        }
        GPixel pColor = convert(paint.getColor());
        GRect adjustedRect;
        adjustedRect.top = rect.top < 0 ? 0 : GRoundToInt(rect.top);
        adjustedRect.left = rect.left < 0 ? 0 : GRoundToInt(rect.left);
        adjustedRect.bottom = rect.bottom > fDevice.height() ? fDevice.height() : GRoundToInt(rect.bottom);
        adjustedRect.right = rect.right > fDevice.width() ? fDevice.width() : GRoundToInt(rect.right);

        for (int y = GRoundToInt(adjustedRect.top); y < adjustedRect.bottom; y++) {
            int L = GRoundToInt(adjustedRect.left);
            int R = GRoundToInt(adjustedRect.right);
            blit(L, R, y, paint, pColor);
        }
    }
    
    std::vector<Edge> getEdges(const GPoint* points, const int edgeCount){
        std::vector<Edge> edges;
        
        for (int i = 0; i < edgeCount - 1; i++) {
            float run = points[i + 1].x - points[i].x;
            float rise = points[i + 1].y - points[i].y;
            if (rise != 0) {
                float m = run / rise;
                float b = points[i].x - m * points[i].y;
                float top = (points[i + 1].y < points[i].y) ? points[i + 1].y : points[i].y;
                float bottom = (points[i + 1].y > points[i].y) ? points[i + 1].y : points[i].y;
                edges.emplace_back(m, b, top, bottom);
            }
        }

        float run = points[edgeCount - 1].x - points[0].x;
        float rise = points[edgeCount - 1].y - points[0].y;
        if (rise != 0) {
            float m = run / rise;
            float b = points[0].x - m * points[0].y;
            float top = (points[edgeCount - 1].y < points[0].y) ? points[edgeCount - 1].y : points[0].y;
            float bottom = (points[edgeCount - 1].y > points[0].y) ? points[edgeCount - 1].y : points[0].y;
            edges.emplace_back(m, b, top, bottom);
        }
        return edges;
    }

    void drawConvexPolygon(const GPoint* points, int pointCount, const GPaint& paint) override {
        GPoint* adjPoints = new GPoint[pointCount];
        ctmStack.top().mapPoints(adjPoints, points, pointCount);
        std::vector<Edge> edges = getEdges(adjPoints, pointCount);
        delete[] adjPoints;
        unsigned int edgeCount = edges.size();
        GPixel pColor = convert(paint.getColor());
        std::sort(edges.begin(), edges.end(), [](Edge& a, Edge& b)
                {
                    return a.top < b.top;
                });

        if (edgeCount == 0) {
            return;
        }
        float start = edges.front().top > 0 ? edges.front().top : 0;
        float end = edges.back().bottom > fDevice.height() ? fDevice.height() : edges.back().bottom;
        unsigned int i = 0;
        unsigned int j = 1;
        
        for (int y = GRoundToInt(start); y < end; y++) {
            if (edges.at(i).bottom <= y) {
                i = std::max(i, j) + 1;
            }
            if (edges.at(j).bottom <= y) {
                j = std::max(i, j) + 1;
            }
            int x1 = GRoundToInt((y + .5f) * edges.at(i).m + edges.at(i).b);
            int x2 = GRoundToInt((y + .5f) * edges.at(j).m + edges.at(j).b);
            x2 = x2 > fDevice.width() ? fDevice.width() : x2;
            x1 = x1 > fDevice.width() ? fDevice.width() : x1;
            x1 = x1 < 0 ? 0 : x1;
            x2 = x2 < 0 ? 0 : x2;
            if (x1 > x2) {
                int temp = x2;
                x2 = x1;
                x1 = temp;
            }
            blit(x1, x2, y, paint, pColor);
        }
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        GPixel pColor = convert(paint.getColor());
        GPath::Edger edger(path);
        GPath::Verb v;
        std::vector<pEdge> edges;
        GPoint pts[GPath::kMaxNextPoints];
        GPoint tPts[GPath::kMaxNextPoints];     // transformed points
        while ((v = edger.next(pts)) != GPath::kDone) {
            ctmStack.top().mapPoints(tPts, pts, GPath::kMaxNextPoints);
            switch (v) {
                case GPath::kLine: {
                    pEdge edge(tPts[0], tPts[1], fDevice.height());
                    if (edge.edgeCheck()) {
                        edges.push_back(edge);
                    }
                }
                break;
                case GPath::kQuad: {
                    int segCount = numSegQuad(tPts);
                    float delta = 1.0f / segCount;
                    GPoint p0, p1;
                    for (float d = 0.0f; d < 1.0f; d += delta) {
                        p0 = p1;
                        if (d + delta >= 1.0f) {
                            p1 = tPts[2];
                        } else {
                            p1 = evaluateQuadratic(tPts, d);
                        }
                        if (d > 0.0f) {
                            pEdge edge(p0, p1, fDevice.height());
                            if (edge.edgeCheck()) {
                                edges.push_back(edge);
                            }
                        }
                    }
                    break;
                }
                case GPath::kCubic: {
                    int segCount = numSegCub(tPts);
                    float delta = 1.0f / segCount;
                    GPoint p0, p1;
                    for (float d = 0.0f; d < 1.0f; d += delta) {
                        p0 = p1;
                        if (d + delta >= 1.0f) {
                            p1 = tPts[3];
                        } else {
                            p1 = evaluateCubic(tPts, d);
                        }
                        if (d > 0.0f) {
                            pEdge edge(p0, p1, fDevice.height());
                            if (edge.edgeCheck()) {
                                edges.push_back(edge);
                            }
                        }
                    }
                    break;
                }
            }
        }
        if (edges.empty()) {
            return;
        }
        GRect bounds = path.bounds();
        if (bounds.top < 0) {
            bounds.top = 0;
        }
        int ymax = edges[0].bottom;
        for (const auto& e : edges) {
            ymax = ymax < e.bottom ? GRoundToInt(e.bottom) : ymax;
        }
        int y = GRoundToInt(bounds.top);
        // y = y < 0 ? 0 : y;
        std::sort(edges.begin(), edges.end(), [](pEdge& a, pEdge& b)
                {
                    return a.top < b.top;
                });
                /*
        std::sort(edges.begin(), edges.end(), [](pEdge& a, pEdge& b) 
                {
                    return a.initX < b.initX;
                });
                */
        size_t i = 0;
        while (i < edges.size() && edgeIsValid(y, edges[i])) {
            //assert(i < edges.size());
            i += 1;
        }
        std::sort(edges.begin(), edges.begin() + i, [y, this](pEdge& a, pEdge& b) 
                {
                    return this->computeX(a.m, a.b, y) < this->computeX(b.m, b.b, y);
                });
        
        for (; y < ymax; ++y) {
            int w = 0;
            i = 0;
            int L;
            while (i < edges.size() && edgeIsValid(y, edges[i])) {
                int x = GRoundToInt(computeX(edges[i].m, edges[i].b, y));

                if (w == 0) {
                    L = x;
                }
                w += edges[i].dir;
                if (w == 0) {
                    int R = x;
                    // draw from L to R
                    // L = L < 0 ? 0 : L;
                    // L = L > fDevice.width() ? fDevice.width() : L;
                    // R = R < 0 ? 0 : R;
                    // R = R > fDevice.width() ? fDevice.width() : R;

                    // call draw method
                    blit(L, R, y, paint, pColor);
                }
                //assert(edges[i].top != 0.344600677f);
                if (edgeIsValid(y + 1, edges[i])) {
                    i += 1;
                } else {
                    edges.erase(edges.begin() + i);
                }
            }
            //assert(w == 0);

            while (i < edges.size() && edgeIsValid(y + 1, edges[i])) {
                //assert(i < edges.size());
                i += 1;
            }

            std::sort(edges.begin(), edges.begin() + i, [y, this](pEdge& a, pEdge& b) 
                    {
                        return this->computeX(a.m, a.b, y + 1) < this->computeX(b.m, b.b, y + 1);
                    });
        }
    }

    GPoint evaluateQuadratic(GPoint points[], float t){
        GPoint res;
        GPoint A = points[0] + (-2) * points[1] + points[2];
        GPoint B = 2 * (points[1] + (-1) * points[0]);
        GPoint C = points[0];
        res = (A * t + B) * t + C;
        return res;
    }

    GPoint evaluateCubic(GPoint points[], float t){
        GPoint res;
        GPoint A = -1 * points[0] + 3 * points[1] + (-3) * points[2] + points[3];
        GPoint B = 3 * points[0] + (-6) * points[1] + 3 * points[2];
        GPoint C = 3 * (points[1] + (-1) * points[0]);
        GPoint D = points[0];
        res = ((A * t + B) * t + C) * t + D;
        return res;
    }

    int numSegQuad(GPoint points[]){
        float tolerance = 0.25;

        GPoint vect = 0.25 * (points[0] + (-2) * points[1] + points[2]);
        float vectMag = sqrt(vect.x * vect.x + vect.y * vect.y);
        return GCeilToInt(sqrt(vectMag / tolerance));
    }

    int numSegCub(GPoint points[]){
        float tolerance = 0.25;

        GPoint vect1 = points[0] + (-2) * points[1] + points[2];
        GPoint vect2 = points[1] + (-2) * points[2] + points[3];

        float vectMag1 = sqrt(vect1.x * vect1.x + vect1.y * vect1.y);
        float vectMag2 = sqrt(vect2.x * vect2.x + vect2.y * vect2.y);

        return GCeilToInt(sqrt(0.75 * std::max(vectMag1, vectMag2) / tolerance));
    }

    bool edgeIsValid(int y, pEdge e) const {
        return y >= e.top && y < e.bottom;
    }

    static float computeX(float m, float b, float y) {
        return m * (y + 0.5f) + b;
    }

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) override {
        // mesh made of triangles defined by triplets of indices
        int n = 0;
        GPoint p0, p1, p2;
        GColor c0, c1, c2;
        GPoint t0, t1, t2;
        std::unique_ptr<GShader> sh;
        std::unique_ptr<GShader> colorSh;
        std::unique_ptr<GShader> texSh;
        GPaint newP = paint;

        for (int i = 0; i < count; ++i) {
            p0 = verts[indices[n]];
            p1 = verts[indices[n+1]];
            p2 = verts[indices[n+2]];
            GPoint pts[] = {p0, p1, p2};

            if (colors != nullptr && texs != nullptr && paint.getShader() != nullptr) {
                c0 = colors[indices[n]];
                c1 = colors[indices[n+1]];
                c2 = colors[indices[n+2]];
                colorSh = GCreateTriangleGradient(p0, p1, p2, c0, c1, c2);
                
                t0 = texs[indices[n]];
                t1 = texs[indices[n+1]];
                t2 = texs[indices[n+2]];

                GMatrix T = GMatrix(t1.x - t0.x, t2.x - t0.x, t0.x, t1.y - t0.y, t2.y - t0.y, t0.y);
                GMatrix P = GMatrix(p1.x - p0.x, p2.x - p0.x, p0.x, p1.y - p0.y, p2.y - p0.y, p0.y);
                GMatrix inverseT;
                T.invert(&inverseT);
                GMatrix fT = GMatrix::Concat(P, inverseT);

                texSh = GCreateTriangleShader(paint.getShader(), fT);

                sh = GCreateCombinedShader(texSh.get(), colorSh.get());

                newP = GPaint(sh.get());
            }

            else if (colors != nullptr) {
                c0 = colors[indices[n]];
                c1 = colors[indices[n+1]];
                c2 = colors[indices[n+2]];
                colorSh = GCreateTriangleGradient(p0, p1, p2, c0, c1, c2);
                newP = GPaint(colorSh.get());
            }

            else if (texs != nullptr && paint.getShader() != nullptr) {
                t0 = texs[indices[n]];
                t1 = texs[indices[n+1]];
                t2 = texs[indices[n+2]];

                GMatrix T = GMatrix(t1.x - t0.x, t2.x - t0.x, t0.x, t1.y - t0.y, t2.y - t0.y, t0.y);
                GMatrix P = GMatrix(p1.x - p0.x, p2.x - p0.x, p0.x, p1.y - p0.y, p2.y - p0.y, p0.y);
                GMatrix inverseT;
                T.invert(&inverseT);
                GMatrix fT = GMatrix::Concat(P, inverseT);

                texSh = GCreateTriangleShader(paint.getShader(), fT);
                newP = GPaint(texSh.get());
            }

            drawConvexPolygon(pts, 3, newP);
            n += 3;
        }
    }
    
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) override {
        GPoint a = verts[0]; GPoint b = verts[1]; GPoint c = verts[2]; GPoint d = verts[3];
        GPoint triVerts[4];
        int indices[6] = {0, 1, 3, 1, 2, 3};

        for (int u = 0; u <= level; u++) {
            for (int v = 0; v <= level; v++) {
                float u1 = (float) u / (float) (level + 1);
                float u2 = (float) (u + 1) / (float) (level +1);
                float v1 = (float) v / (float) (level + 1);
                float v2 = (float) (v + 1) / (float) (level +1);

                triVerts[0] = quadPoint(a, b, c, d, u1, v1);
                triVerts[1] = quadPoint(a, b, c, d, u2, v1);
                triVerts[2] = quadPoint(a, b, c, d, u2, v2);
                triVerts[3] = quadPoint(a, b, c, d, u1, v2);
                GColor* colorPtr = nullptr;
                GPoint* texPtr = nullptr;
                
                GColor adjColors[4];
                if (colors) {
                    adjColors[0] = quadColor(colors[0], colors[1], colors[2], colors[3], u1, v1);
                    adjColors[1] = quadColor(colors[0], colors[1], colors[2], colors[3], u2, v1);
                    adjColors[2] = quadColor(colors[0], colors[1], colors[2], colors[3], u2, v2);
                    adjColors[3] = quadColor(colors[0], colors[1], colors[2], colors[3], u1, v2);
                    colorPtr = adjColors;
                }

                GPoint adjTexs[4];
                if (texs) {
                    adjTexs[0] = quadPoint(texs[0], texs[1], texs[2], texs[3], u1, v1);
                    adjTexs[1] = quadPoint(texs[0], texs[1], texs[2], texs[3], u2, v1);
                    adjTexs[2] = quadPoint(texs[0], texs[1], texs[2], texs[3], u2, v2);
                    adjTexs[3] = quadPoint(texs[0], texs[1], texs[2], texs[3], u1, v2);
                    texPtr = adjTexs;
                }

                drawMesh(triVerts, colorPtr, texPtr, 2, indices, paint);
            }
        }
    }

    void drawZigZag(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
        GPoint a = verts[0]; GPoint b = verts[1]; GPoint c = verts[2]; GPoint d = verts[3];
        GPoint triVerts[4];
        int indices[6] = {0, 1, 3, 1, 2, 3};
        GColor* colorPtr = nullptr;
        GPoint* texPtr = nullptr;

        for (int u = 0; u <= level; u++) {
            for (int v = 0; v <= level; v++) {
                float u1 = (float) u / (float) (level + 1);
                float u2 = (float) (u + 1) / (float) (level +1);
                float v1 = (float) v / (float) (level + 1);
                float v2 = (float) (v + 1) / (float) (level +1);

                triVerts[0] = quadPoint(a, b, c, d, u1, v1);
                triVerts[1] = quadPoint(a, b, c, d, v2, v1);
                triVerts[2] = quadPoint(a, b, c, d, u2, v2);
                triVerts[3] = quadPoint(a, b, c, d, v1, v2);

                GColor adjColors[4];
                if (colors) {
                    adjColors[0] = quadColor(colors[0], colors[1], colors[2], colors[3], u1, v1);
                    adjColors[1] = quadColor(colors[0], colors[1], colors[2], colors[3], u2, v1);
                    adjColors[2] = quadColor(colors[0], colors[1], colors[2], colors[3], u2, v2);
                    adjColors[3] = quadColor(colors[0], colors[1], colors[2], colors[3], u1, v2);
                    colorPtr = adjColors;
                }

                GPoint adjTexs[4];
                if (texs) {
                    adjTexs[0] = quadPoint(texs[0], texs[1], texs[2], texs[3], u1, v1);
                    adjTexs[1] = quadPoint(texs[0], texs[1], texs[2], texs[3], u2, v1);
                    adjTexs[2] = quadPoint(texs[0], texs[1], texs[2], texs[3], u2, v2);
                    adjTexs[3] = quadPoint(texs[0], texs[1], texs[2], texs[3], u1, v2);
                    texPtr = adjTexs;
                }

                drawMesh(triVerts, colorPtr, texPtr, 2, indices, paint);
            }
        }
    }
    
    GPoint quadPoint(GPoint a, GPoint b, GPoint c, GPoint d, float u, float v) {
        return (1.0f - u) * (1.0f - v) * a + (1.0f - v) * u * b + (1.0f - u) * v * d + u * v * c;
    }

    GColor quadColor(GColor A, GColor B, GColor C, GColor D, float u, float v) {
        return (1.0f - u) * (1.0f - v) * A + (1.0f - v) * u * B + (1.0f - u) * v * D + u * v * C;
    }

    void blit (int L, int R, int y, GPaint paint, GPixel pColor) {
        L = L < 0 ? 0 : L;
        L = L > fDevice.width() ? fDevice.width() : L;
        R = R < 0 ? 0 : R;
        R = R > fDevice.width() ? fDevice.width() : R;
        if (R == L) {
            return;
        }
        std::vector<GPixel> storage(R - L);

        switch (paint.getBlendMode()) {
            case GBlendMode::kClear:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kClear(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kClear(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kSrc:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kSrc(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kSrc(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kDst:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kDst(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kDst(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kSrcOver:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kSrcOver(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kSrcOver(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kDstOver:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kDstOver(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kDstOver(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kSrcIn:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kSrcIn(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kSrcIn(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kDstIn:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kDstIn(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kDstIn(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kSrcOut:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kSrcOut(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kSrcOut(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kDstOut:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kDstOut(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kDstOut(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kSrcATop:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kSrcATop(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kSrcATop(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kDstATop:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kDstATop(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kDstATop(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
            case GBlendMode::kXor:
                if (paint.getShader() != nullptr) {
                    if (paint.getShader()->setContext(ctmStack.top())) {
                        paint.getShader()->shadeRow(L, y, R - L, &storage[0]);
                        for (int p = L; p < R; p++) {
                            GPixel *dest = fDevice.getAddr(p, y);
                            *dest = kXor(storage[p-L], *dest);
                        }
                    }
                }
                else {
                    for (int p = L; p < R; p++) {
                        GPixel *dest = fDevice.getAddr(p, y);
                        GPixel blended = kXor(pColor, *dest);
                        *fDevice.getAddr(p, y) = blended;
                    }
                }
                break;
        }
    }

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    std::stack<GMatrix> ctmStack;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

GPath addWeirdFlareyShape(GPoint center, float radius /*, GPath::Direction dir*/) {
    float tan = tanf(gFloatPI / 8.0f);
    float root = sqrt(2.0f) / 2.0f;

    GPoint pts[16];
    pts[0] = {1, 0};
    pts[1] = {1, tan};
    pts[2] = {root, root};
    pts[3] = {tan, 1};
    pts[4] = {0, 1};
    pts[5] = {-tan, 1};
    pts[6] = {-root, root};
    pts[7] = {-1, tan};
    pts[8] = {-1, 0};
    pts[9] = {-1, -tan};
    pts[10] = {-root, -root};
    pts[11] = {-tan, -1};
    pts[12] = {0, -1};
    pts[13] = {tan, -1};
    pts[14] = {root, -root};
    pts[15] = {1, -tan};

    GMatrix trans = GMatrix::Translate(center.x, center.y);
    GMatrix scale = GMatrix::Scale(radius, radius);
    GMatrix mtx = GMatrix::Concat(trans, scale);

    mtx.mapPoints(pts, 16);
    GPath path = GPath();

    path.moveTo(pts[0]);
    // for (int i = 0; i < 4; ++i) {
    //     quadTo(pts[i + 1], pts[i + 2]);
    //     quadTo(pts[i + 3], pts[i + 4]);
    // }
    // if (dir == GPath::kCCW_Direction) {
    //     for (int i = 14; i >= 0; i -= 2) {
    //         path.quadTo(pts[i], pts[i + 1]);
    //     }
    // } else {
        for (int i = 2; i < 16; i += 2) {
            path.quadTo(pts[i], pts[i - 1]);
        }
    // }

    //path.transform(mtx);
    //assert(radius != 10);
    return path;
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    // as fancy as you like
    // ...
    // canvas->clear(...);
    // canvas->fillRect(...);
    canvas->clear(GColor::RGB(0, .3, .4));
    float fRadius = 16.0f;
    canvas->save();
    GPaint darkPaint = GPaint({0.0f, 0.476f, 0.76f, 1.0f});
    GPaint lightPaint = GPaint({0.0f, 0.4f, 0.5f, 1.0f});
    for (float x = fRadius - 1; x <= dim.width - fRadius; x += (fRadius*2)) {
        for (float y = (0.0f); y <= dim.width; y += (fRadius*2)) {
            GPath darkFlare = addWeirdFlareyShape({0.0f, 0.0f}, fRadius);
            canvas->translate(x, y);
            canvas->drawPath(darkFlare, darkPaint);
            canvas->restore();
            canvas->save();
        }
    }
    float fRadius2 = 12.0f;
    for (float x = 0.0f; x <= dim.width; x += (fRadius*2)) {
        for (float y = (fRadius - 1); y <= dim.width - fRadius; y += (fRadius*2)) {
            GPath lightFlare = addWeirdFlareyShape({0.0f, 0.0f}, fRadius2);
            canvas->translate(x, y);
            canvas->drawPath(lightFlare, lightPaint);
            canvas->restore();
            canvas->save();
        }
    }

    
    GPath path = GPath();
    path.addCircle({128, 128}, 80, GPath::kCW_Direction);
    //assert(path.bounds().top == 3);
    GPaint red = GPaint({1.0f, 0.3f, 0.2f, 1.0f});
    canvas->drawPath(path, red);
    /*
    GPaint paint = GPaint(GColor::RGBA(0.5, 0.5, 0.5, 1));

    canvas->save();
    canvas->rotate(-gFloatPI/3);
    canvas->drawRect(GRect::XYWH(20, 20, 100, 100), paint);
    canvas->translate(10, 160);
    GPoint points[4];
    points[0].x = 20; points[0].y = 20;
    points[1].x = 20; points[1].y = 120;
    points[2].x = 120; points[2].y = 20;
    points[3].x = 120; points[3].y = 120;
    canvas->drawConvexPolygon(points, 4, paint);
    */

    return "Shlawoop";
}

