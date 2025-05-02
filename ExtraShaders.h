#include "include/GShader.h"

std::unique_ptr<GShader> GCreateTriangleShader(GShader* shader, GMatrix& mtx);
std::unique_ptr<GShader> GCreateTriangleGradient(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2);
std::unique_ptr<GShader> GCreateCombinedShader(GShader* sh1, GShader* sh2);