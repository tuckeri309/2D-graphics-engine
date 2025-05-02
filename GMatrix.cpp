#include "include/GMatrix.h"
#include "include/GPoint.h"

GMatrix::GMatrix() {
    fMat[0] = 1.0f; fMat[1] = 0.0f; fMat[2] = 0.0f; fMat[3] = 0.0f; fMat[4] = 1.0f; fMat[5] = 0.0f;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1.0f, 0.0f, tx, 0.0f, 1.0f, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0.0f, 0.0f, 0.0f, sy, 0.0f);
}

GMatrix GMatrix::Rotate(float radians) {
    float cosTheta = cosf(radians);
    float sinTheta = sinf(radians);
    return GMatrix(cosTheta, -sinTheta, 0.0f, sinTheta, cosTheta, 0.0f);
}

GMatrix multMatrices(const GMatrix& M, const GMatrix& m){
    GMatrix mult;
    float a = M[0]*m[0]+M[1]*m[3];
    float b = M[0]*m[1]+M[1]*m[4];
    float c = M[0]*m[2]+M[1]*m[5]+M[2];
    float d = M[3]*m[0]+M[4]*m[3];
    float e = M[3]*m[1]+M[4]*m[4];
    float f = M[3]*m[2]+M[4]*m[5]+M[5];
    return GMatrix(a,b,c,d,e,f);
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    GMatrix temp = multMatrices(a, b);
    return temp;
}

bool GMatrix::invert(GMatrix* inverse) const {
    float determinant = (*this)[0] * (*this)[4] - (*this)[1] * (*this)[3];
    if (determinant == 0) {
        return false;
    }
    determinant = 1.0f / determinant;
    float inverseResult[6] = {
        (*this)[4] * determinant, -1.0f * (*this)[1] * determinant, ((*this)[1] * (*this)[5] - (*this)[2] * (*this)[4]) * determinant,
        -1.0f * (*this)[3] * determinant, (*this)[0] * determinant, -1.0f * ((*this)[0] * (*this)[5] - (*this)[2] * (*this)[3]) * determinant
    };

    *inverse = GMatrix(inverseResult[0], inverseResult[1], inverseResult[2], inverseResult[3], inverseResult[4], inverseResult[5]);
    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; i++) {
        float x = src[i].x;
        float y = src[i].y;
        dst[i].x = (*this)[0] * x + (*this)[1] * y + (*this)[2];
        dst[i].y = (*this)[3] * x + (*this)[4] * y + (*this)[5];
    }
}