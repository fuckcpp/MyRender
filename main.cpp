#include <cmath>
#include "tgaimage.h"
#include "Model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model* model;
int width = 300;
int height = 200;

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

void drawModel(Model* model, TGAImage& image)
{
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f v1 = model->Vert(face[j]);
			Vec3f v2 = model->Vert(face[(j + 1) % 3]);

			Vec2i vi0, vi1;
			vi0.x = (v1.x + 1.) * width / 2;
			vi0.y = (v1.y + 1.) * height / 2;
			vi1.x = (v2.x + 1.) * width / 2;
			vi1.y = (v2.y + 1.) * height / 2;
			line(vi0, vi1, image, white);
		}
	}
}

void triangle(Vec2i t0,Vec2i t1,Vec2i t2, TGAImage &image,TGAColor color)
{
	line(t0, t1, image, color);
	line(t1, t2, image, color);
	line(t2, t0, image, color);
}


int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

	/*for (int i = 0; i < 1000000; i++) {
		line(13, 20, 80, 40, image, white);
		line(20, 13, 40, 80, image, red);
		line(80, 40, 13, 20, image, red);
	}*/

	//model = new Model("obj/african_head.obj");
	//drawModel(model,image);

	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}
