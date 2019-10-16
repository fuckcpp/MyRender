#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include "tgaimage.h"
#include "Model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model* model;
const int width = 800;
const int height = 500;
const int depth = 255;
int* zbuffer;


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


void triangle(Vec3i t0,Vec3i t1,Vec3i t2, TGAImage &image,TGAColor color)
{
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int seg_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / seg_height; // be careful: with above conditions no division by zero here
		Vec3i A = t0 + (t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) {
			float alpha = B.x == A.x ? 1. : ( j-A.x ) / (float)(B.x - A.x);
			Vec3i P;
			P.x = j;
			P.y = t0.y + i;
			P.z= A.z * (1 - alpha) + B.z * alpha;
			if (zbuffer[P.y * width + P.x] < P.z)//必须做比较
			{
				zbuffer[P.y * width + P.x] = P.z;
				image.set(P.x, P.y, color); // attention, due to int casts t0.y+i != A.y
			}
		}
		
	}
}

void drawModel(Model* model, TGAImage& image)
{
	Vec3f lightDir = { 0,0,-1 };
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3i screen_coords[3];
		Vec3f world_pos[3];
		float intensity;
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->Vert(face[j]);
			screen_coords[j].x = (v.x + 1.) * width / 2;
			screen_coords[j].y = (v.y + 1.) * height / 2;
			screen_coords[j].z = (v.z + 1.) * depth / 2;
			world_pos[j] = v;
		}
		//光照计算
		Vec3f normal = (world_pos[2] - world_pos[0]) ^ (world_pos[1] - world_pos[0]);
		intensity = normal.normlize()*lightDir;
		if (intensity > 0)//back_culling 剔除反面的三角形
		{
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
}


int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

	TGAImage zbufferImage(width, height, TGAImage::GRAYSCALE);
	zbuffer = new int[width*height];
	for (int i = 0; i < width *height; i++)
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	model = new Model("obj/african_head.obj");
	drawModel(model,image);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			zbufferImage.set(i,j, TGAColor(zbuffer[i+width*j],1));
	}

	zbufferImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbufferImage.write_tga_file("zbuffer.tga");
	delete model;
	return 0;
}
