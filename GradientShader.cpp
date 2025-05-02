#include "include/GShader.h"
#include "include/GMatrix.h"
#include <vector>


class LinearGradient : public GShader {
public:
    LinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, TileMode mode){
        if (p0.x < p1.x) {
            fP0 = p0;
            fP1 = p1;

        } else if (p0.x == p1.x && p0.y < p1.y) {
            fP0 = p0;
            fP1 = p1;
        } else {
            fP0 = p1;
            fP1 = p0;
        }

        fColorCount = count;
        for (int i = 0; i < fColorCount; i++) {
            fColors.push_back(colors[i]);
        }

        tileMode = mode;
    }

    bool isOpaque() override{
        return false;
    }

    bool setContext(const GMatrix& ctm) override{
        float dx = fP1.x - fP0.x;
        float dy = fP1.y - fP0.y;
        fLocalMatrix = GMatrix(dx, -dy, fP0.x ,dy, dx, fP0.y);

        fLocalMatrix.invert(&gInverse);
        fInverse = GMatrix();

        if (!ctm.invert(&fInverse)) {
            return false;
        }

        ctm.invert(&fInverse);
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint p;
        p.x = fInverse.operator[](0) * (x + 0.5f) + fInverse.operator[](1) * (y + 0.5f) + fInverse.operator[](2);
        p.y = fInverse.operator[](3) * (x + 0.5f) + fInverse.operator[](4) * (y + 0.5f) + fInverse.operator[](5);
        for (int i = 0; i < count; ++i)
        {
            GColor c;
            if (fColorCount == 1) {
                c = fColors.front();
            }
            else {
                float t = gInverse.operator[](0) * (p.x) + gInverse.operator[](1) * (p.y) + gInverse.operator[](2);
                if (tileMode==kClamp) {
                    t = std::max(0.0f, std::min(1.0f, t));
                } else if (tileMode == kRepeat) {
                    t = t - GFloorToInt(t);
                } else if(tileMode == kMirror) {
                    t *= 0.5;
                    t -= floor(t);
                    if (t > 0.5) {
                        t = 1 - t;
                    }
                    t *= 2;
                }

                int num_intervals = fColors.size()-1;
                int index = t * num_intervals;
                float u = (t - (1.0/num_intervals)*index)*num_intervals;

                GColor c1, c2;

                c1 = fColors[index+1];
                c2 = fColors[index];

                c.a = c1.a * u + c2.a * (1-u);
                c.r = c1.r * u + c2.r * (1-u);
                c.g = c1.g * u + c2.g * (1-u);
                c.b = c1.b * u + c2.b * (1-u);
            }

            row[i] = colorToPixel(c);
            p.x += fInverse[0];
            p.y += fInverse[3];
        }


    }

private:
    GPoint fP0;
    GPoint fP1;

    std::vector<GColor> fColors;
    int fColorCount;

    GMatrix fLocalMatrix;
    GMatrix fInverse;
    GMatrix gInverse;

    TileMode tileMode;

    int floatToInt255(float f){
        return GFloorToInt(f * 255 + 0.5);
    }

    GPixel colorToPixel(const GColor& color){
        return GPixel_PackARGB(floatToInt255(color.a), floatToInt255(color.r*color.a),floatToInt255(color.g*color.a), floatToInt255(color.b*color.a));
    }

};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode) {
    return std::unique_ptr<GShader>(new LinearGradient(p0, p1, colors, count, mode));
}