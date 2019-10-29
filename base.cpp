#include "base.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

extern std::vector<int> zbuffer(width* height, std::numeric_limits<int>::min());
extern Vec3f lightDir = { 0,0,-1 };
extern Vec3f camera = { 0,0,3 };

Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (std::abs(v0.x - v1.x) < std::abs(v0.y - v1.y)) { // if the line is steep, we transpose the image 
		std::swap(v0.x, v0.y);
		std::swap(v1.x, v1.y);
		steep = true;
	}
	if (v0.x > v1.x) { // make it left−to−right 
		std::swap(v0.x, v1.x);
		std::swap(v0.y, v1.y);
	}

	int dy = v1.y - v0.y;
	int dx = v1.x - v0.x;
	int dy2 = std::abs(dy) * 2;
	int yadd = 0;
	int y = v0.y;
	for (int x = v0.x; x <= v1.x; x++) {
		if (steep) {
			image.set(y, x, color); // if transposed, de−transpose 
		}
		else {
			image.set(x, y, color);
		}
		yadd += dy2;
		if (yadd > dx)
		{
			y += v1.y > v0.y ? 1 : -1;
			yadd -= dx * 2;
		}
	}
}

void triangle(Vec3i* t, TGAImage& image, TGAColor color)
{
	if (t[0].y == t[1].y && t[0].y == t[2].y) return; // i dont care about degenerate triangles
	if (t[0].y > t[1].y) std::swap(t[0], t[1]);
	if (t[0].y > t[2].y) std::swap(t[0], t[2]);
	if (t[1].y > t[2].y) std::swap(t[1], t[2]);
	int total_height = t[2].y - t[0].y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t[1].y - t[0].y || t[1].y == t[0].y;
		int seg_height = second_half ? t[2].y - t[1].y : t[1].y - t[0].y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t[1].y - t[0].y : 0)) / seg_height; // be careful: with above conditions no division by zero here
		Vec3i A = t[0] + (t[2] - t[0]) * alpha;
		Vec3i B = second_half ? t[1] + (t[2] - t[1]) * beta : t[0] + (t[1] - t[0]) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) {
			float alpha = B.x == A.x ? 1. : (j - A.x) / (float)(B.x - A.x);
			Vec3i P;
			P.x = j;
			P.y = t[0].y + i;
			P.z = A.z * (1 - alpha) + B.z * alpha;
			if (zbuffer[P.y * width + P.x] < P.z)//必须做比较
			{
				zbuffer[P.y * width + P.x] = P.z;
				image.set(P.x, P.y, color); // attention, due to int casts t[0].y+i != A.y
			}
		}

	}
}

void triangle_frag(Vec3i* t, Vec2i* uv, TGAImage& tex, float intensity, TGAImage& image)
{
	if (t[0].y == t[1].y && t[0].y == t[2].y) return; // i dont care about degenerate triangles
	if (t[0].y > t[1].y) { std::swap(t[0], t[1]); std::swap(uv[0], uv[1]); }
	if (t[0].y > t[2].y) { std::swap(t[0], t[2]); std::swap(uv[0], uv[2]); }
	if (t[1].y > t[2].y) { std::swap(t[1], t[2]); std::swap(uv[1], uv[2]); }
	int total_height = t[2].y - t[0].y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t[1].y - t[0].y || t[1].y == t[0].y;
		int seg_height = second_half ? t[2].y - t[1].y : t[1].y - t[0].y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t[1].y - t[0].y : 0)) / seg_height; // be careful: with above conditions no division by zero here
		Vec3i A = t[0] + (t[2] - t[0]) * alpha;
		Vec2i uvA = uv[0] + (uv[2] - uv[0]) * alpha;
		Vec3i B = second_half ? t[1] + (t[2] - t[1]) * beta : t[0] + (t[1] - t[0]) * beta;
		Vec2i uvB = second_half ? uv[1] + (uv[2] - uv[1]) * beta : uv[0] + (uv[1] - uv[0]) * beta;
		if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); }
		for (int j = A.x; j <= B.x; j++) {
			float alpha = B.x == A.x ? 1. : (j - A.x) / (float)(B.x - A.x);
			Vec3i P;
			Vec2i UV;
			P.x = j;
			UV.x = uvA.x + alpha * (uvB.x - uvA.x);
			P.y = t[0].y + i;
			UV.y = uvA.y + alpha * (uvB.y - uvA.y);
			P.z = A.z * (1 - alpha) + B.z * alpha;
			TGAColor color = tex.get(UV.x, UV.y);
			int index = P.y * width + P.x;
			if (0 <= index && index < width * height)//防止顶点坐标超出视口范围，导致深度缓冲越界
			{
				if (zbuffer[index] < P.z)//必须做比较
				{
					zbuffer[index] = P.z;
					image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 1.f)); // attention, due to int casts t[0].y+i != A.y
				}
			}
		}
	}
}