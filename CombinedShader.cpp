#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GPoint.h"
#include "ExtraShaders.h"

class CombinedShader : public GShader {
public:
    CombinedShader(GShader* _sh1, GShader* _sh2) {
        sh1 = _sh1;
        sh2 = _sh2;
    }

    bool isOpaque() override {
        return sh1->isOpaque() && sh2->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        return sh1->setContext(ctm) && sh2->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel tmp[count];
        sh1->shadeRow(x, y, count, row);
        sh2->shadeRow(x, y, count, tmp);

        for (int i = 0; i < count; i++) {
            row[i] = GPixel_PackARGB(
                // (GPixel_GetA(row[i]) * GPixel_GetA(tmp[i]) * 257) >> 16,
                // (GPixel_GetR(row[i]) * GPixel_GetR(tmp[i]) * 257) >> 16,
                // (GPixel_GetG(row[i]) * GPixel_GetG(tmp[i]) * 257) >> 16,
                // (GPixel_GetB(row[i]) * GPixel_GetB(tmp[i]) * 257) >> 16
                GRoundToInt(GPixel_GetA(row[i]) * GPixel_GetA(tmp[i]) / 255.0f),
                GRoundToInt(GPixel_GetR(row[i]) * GPixel_GetR(tmp[i]) / 255.0f),
                GRoundToInt(GPixel_GetG(row[i]) * GPixel_GetG(tmp[i]) / 255.0f),
                GRoundToInt(GPixel_GetB(row[i]) * GPixel_GetB(tmp[i]) / 255.0f)
            );
        }
    }
private:
    GShader* sh1;
    GShader* sh2;
};

std::unique_ptr<GShader> GCreateCombinedShader(GShader* sh1, GShader* sh2) {
    return std::unique_ptr<GShader>(new CombinedShader(sh1, sh2));
}