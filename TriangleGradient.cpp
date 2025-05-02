#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "ExtraShaders.h"
#include <vector>


class TriangleGradient : public GShader {
public:
	TriangleGradient(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2){
		fP0 = p0; fP1 = p1; fP2 = p2;
		fC0 = c0; fC1 = c1; fC2 = c2;
	}

	bool isOpaque() override{
		return false;
	}

	bool setContext(const GMatrix& ctm) override{
		GPoint U = fP1-fP0;
		GPoint V = fP2-fP0;
		fLocalMatrix = GMatrix(U.x, V.x, fP0.x, U.y, V.y, fP0.y);

		dc0 = GColor::RGBA(fC1.r - fC0.r, fC1.g - fC0.g, fC1.b - fC0.b, fC1.a - fC0.a);
		dc1 = GColor::RGBA(fC2.r - fC0.r, fC2.g - fC0.g, fC2.b - fC0.b, fC2.a - fC0.a);

		fLocalMatrix.invert(&gInverse);
		fInverse = GMatrix();

		if (!ctm.invert(&fInverse)) {
			return false;
		}

		ctm.invert(&fInverse);

		P = GMatrix::Concat(gInverse, fInverse);
		return true;
	}

	void shadeRow(int x, int y, int count, GPixel row[]) override{
		GPoint p;
		p.x = fInverse.operator[](0) * (x + 0.5f) + fInverse.operator[](1) * (y + 0.5f) + fInverse.operator[](2);
		p.y = fInverse.operator[](3) * (x + 0.5f) + fInverse.operator[](4) * (y + 0.5f) + fInverse.operator[](5);
		float fx = gInverse.operator[](0) * (p.x) + gInverse.operator[](1) * (p.y) + gInverse.operator[](2);
		float fy = gInverse.operator[](3) * (p.x) + gInverse.operator[](4) * (p.y) + gInverse.operator[](5);
		GColor c;
		c.a = fx*dc0.a + fy*dc1.a + fC0.a;
		c.r = fx*dc0.r + fy*dc1.r + fC0.r;
		c.g = fx*dc0.g + fy*dc1.g + fC0.g;
		c.b = fx*dc0.b + fy*dc1.b + fC0.b;
		c.a = std::max(0.0f, std::min(1.0f, c.a));
		c.r = std::max(0.0f, std::min(1.0f, c.r));
		c.g = std::max(0.0f, std::min(1.0f, c.g));
		c.b = std::max(0.0f, std::min(1.0f, c.b));

		row[0] = colorToPixel(c);

		GColor dc;
		dc.a = P[0]*dc0.a + P[3]*dc1.a ;
		dc.r = P[0]*dc0.r + P[3]*dc1.r ;
		dc.g = P[0]*dc0.g + P[3]*dc1.g ;
		dc.b = P[0]*dc0.b + P[3]*dc1.b ;


		for (int i = 1; i < count; ++i){
			GColor nc;
			nc.a = c.a + dc.a ;
			nc.r = c.r + dc.r ;
			nc.g = c.g + dc.g ;
			nc.b = c.b + dc.b ;

			nc.a = std::max(0.0f, std::min(1.0f, nc.a));
			nc.r = std::max(0.0f, std::min(1.0f, nc.r));
			nc.g = std::max(0.0f, std::min(1.0f, nc.g));
			nc.b = std::max(0.0f, std::min(1.0f, nc.b));
			assert(nc.a != 1.0f || nc.r != 1.0f || nc.g != 1.0f || nc.b != 1.0f);

			row[i] = colorToPixel(nc);

			c = nc;
		}
  	}

private:
    GPoint fP0;
    GPoint fP1;
    GPoint fP2;

    GColor fC0;
    GColor fC1;
    GColor fC2;

    GColor dc0, dc1;

    GMatrix fLocalMatrix;
    GMatrix fInverse;
    GMatrix gInverse;
    GMatrix P;

    int floatToInt255(float f){
        return GFloorToInt(f * 255 + 0.5);
    }

    GPixel colorToPixel(const GColor& color){
        return GPixel_PackARGB(floatToInt255(color.a), floatToInt255(color.r*color.a),floatToInt255(color.g*color.a), floatToInt255(color.b*color.a));
    }
};

std::unique_ptr<GShader> GCreateTriangleGradient(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2) {
    return std::unique_ptr<GShader>(new TriangleGradient(p0, p1, p2, c0, c1, c2));
}