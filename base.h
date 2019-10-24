#pragma once
#include<vector>
#include "geometry.h"
#include "tgaimage.h"
const int width = 800;
const int height = 500;
const int depth = 255;
extern std::vector<int> zbuffer;
void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color);
void triangle(Vec3i* t, TGAImage& image, TGAColor color);
void triangle_frag(Vec3i* t, Vec2i* uv, TGAImage& tex, float intensity, TGAImage& image);