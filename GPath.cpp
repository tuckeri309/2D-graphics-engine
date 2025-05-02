#include "include/GPath.h"

void GPath::addRect(const GRect& r, Direction dir) {
    switch (dir) {
        case (kCW_Direction):
            moveTo({r.left, r.top});
            lineTo({r.right, r.top});
            lineTo({r.right, r.bottom});
            lineTo({r.left, r.bottom});
            break;
        case (kCCW_Direction):
            moveTo({r.left, r.top});
            lineTo({r.left, r.bottom});
            lineTo({r.right, r.bottom});
            lineTo({r.right, r.top});
    }

}

void GPath::addPolygon(const GPoint pts[], int count) {
    moveTo(pts[0]);
    for (int i = 1; i < count; i++) {
        lineTo(pts[i]);
    }
}

GRect GPath::bounds() const {
    if (fPts.empty()) {
        return GRect::XYWH(0, 0, 0, 0);
    }
    float maxX = fPts[0].x;
    float maxY = fPts[0].y;
    float minX = fPts[0].x;
    float minY = fPts[0].y;
    for (int i = 1; i < fPts.size(); i++) {
        if (fPts[i].x > maxX) {
            maxX = fPts[i].x;
        }
        if (fPts[i].y > maxY) {
            maxY = fPts[i].y;
        }
        if (fPts[i].x < minX) {
            minX = fPts[i].x;
        }
        if (fPts[i].y < minY) {
            minY = fPts[i].y;
        }
    }

    return GRect::LTRB(minX, minY, maxX, maxY);
}

void GPath::transform(const GMatrix& m) {
    for (int i = 0; i < fPts.size(); i++) {
        float x = fPts[i].x;
        float y = fPts[i].y;
        fPts[i].x = m[0] * x + m[1] * y + m[2];
        fPts[i].y = m[3] * x + m[4] * y + m[5];
    }
}

void GPath::addCircle(GPoint center, float radius, Direction dir) {
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

    GMatrix matrix = GMatrix::Concat(GMatrix::Translate(center.x, center.y), GMatrix::Scale(radius, radius));
    matrix.mapPoints(pts, 16);

    moveTo(pts[0]);
    for (int i = 0; i < 8; i++) {
        if (dir == GPath::kCW_Direction) {
            quadTo(pts[i*2+1], i != 7 ? pts[i*2+2] : pts[0]);
        } else {
            quadTo(pts[15-i*2], pts[14-i*2]);
        }
    }

    //transform(matrix);
    //assert(radius != 10);
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    dst[0] = src[0];
    dst[4] = src[2];
    dst[1] = (1 - t) * src[0] + t * src[1];
    dst[3] = (1 - t) * src[1] + t * src[2];
    dst[2] = (1 - t) * dst[1] + t * dst[3];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    dst[0] = src[0];
    dst[1] = (1 - t) * src[0] + t * src[1];
    dst[5] = (1 - t) * src[2] + t * src[3];
    dst[6] = src[3];

    GPoint inter = (1 - t) * src[1] + t * src[2];
    dst[2] = (1 - t) * dst[1] + t * inter;
    dst[4] = (1 - t) * inter + t * dst[5];
    dst[3] = (1 - t) * dst[2] + t * dst[4];
}