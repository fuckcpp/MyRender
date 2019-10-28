#pragma once
#include<vector>
#include "geometry.h"
#include "tgaimage.h"
const int width = 800;
const int height = 500;
const int depth = 255;
extern std::vector<int> zbuffer;
extern Vec3f lightDir;
extern Vec3f camera;

//矩阵转向量
Vec3f m2v(Matrix m);
//向量转矩阵
Matrix v2m(Vec3f v);
//临时视口
Matrix viewport(int x, int y, int w, int h);

void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color);
void triangle(Vec3i* t, TGAImage& image, TGAColor color);
void triangle_frag(Vec3i* t, Vec2i* uv, TGAImage& tex, float intensity, TGAImage& image);