#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include <iostream>
#include <algorithm>
#include <stack>

class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localM, TileMode mode) : fBitmap(bitmap), fLocalMatrix(localM), tileMode(mode){
    }

    bool isOpaque() override{
        return this->fBitmap.isOpaque();
    }

    bool setContext(const GMatrix& ctm) override{
        if (!ctm.invert(&fInverse)) {
            return false;
        }

        // fLocalMatrix is already inverted
        fInverse = fInverse.Concat(fLocalMatrix, fInverse);

        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint p;
        p.x = fInverse.operator[](0) * (x + 0.5f) + fInverse.operator[](1) * (y + 0.5f) + fInverse.operator[](2);
        p.y = fInverse.operator[](3) * (x + 0.5f) + fInverse.operator[](4) * (y + 0.5f) + fInverse.operator[](5);

        for (int i = 0; i < count; ++i) {

            float sourceX = p.x/fBitmap.width();
            float sourceY = p.y/fBitmap.height();

            if (tileMode == kClamp) {
                sourceX = std::min(std::max(sourceX, 0.0f), 0.9999f);
                sourceY = std::min(std::max(sourceY,0.0f), 0.9999f);
            } else if (tileMode == kRepeat) {
                sourceX = sourceX - GFloorToInt(sourceX);
                sourceY = sourceY - GFloorToInt(sourceY);
            } else if (tileMode == kMirror) {
                sourceX *= 0.5;
                sourceY *= 0.5;
                sourceX -= floor(sourceX);
                sourceY -= floor(sourceY);
                if (sourceX>0.5) {
                    sourceX = 1 - sourceX;
                }
                if (sourceY>0.5) {
                    sourceY = 1 - sourceY;
                }
                sourceX *= 2;
                sourceY *= 2;
            }

            sourceX = std::min(std::max(sourceX, 0.0f), 1.0f);
            sourceY = std::min(std::max(sourceY, 0.0f), 1.0f);

            int isourceX = GFloorToInt(sourceX * fBitmap.width());
            int isourceY = GFloorToInt(sourceY * fBitmap.height());

            isourceX = std::max(0, std::min(fBitmap.width()-1, isourceX));
            isourceY = std::max(0, std::min(fBitmap.height()-1, isourceY));

            row[i] = *fBitmap.getAddr(isourceX, isourceY);

            p.x += fInverse[0];
            p.y += fInverse[3];

        }

    }

private:
    TileMode tileMode;
    GBitmap fBitmap;
    GMatrix fLocalMatrix; // this should already be inverted when passed into shader
    GMatrix fInverse;

};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localM, BitmapShader::TileMode mode) {
    if (!bitmap.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GShader>(new BitmapShader(bitmap, localM, mode));
};