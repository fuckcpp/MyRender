#include <cmath>
#include "tgaimage.h"
#include "Model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model;
int width = 800;
int height = 600;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) { // if the line is steep, we transpose the image 
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) { // make it left−to−right 
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	
	int dy=y1-y0;
	int dx=x1-x0;
	int dy2=std::abs(dy)*2;
	int yadd=0;
	int y=y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color); // if transposed, de−transpose 
		}
		else {
			image.set(x, y, color);
		}
		yadd+=dy2;
		if(yadd>dx)
		{
			y+=y1>y0?1:-1;	
			yadd-=dx*2;
		}
	}
}
int main(int argc, char** argv) {

	model = new Model("obj/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f v1 = model->Vert(face[j]);
			Vec3f v2 = model->Vert(face[(j+1)%3]);
			int x0 = (v1.x + 1.) * width /2;
			int y0 = (v1.y + 1.) * height/2;
			int x1 = (v2.x + 1.) * width /2;
			int y1 = (v2.y + 1.) * height /2;
			line(x0,y0,x1,y1, image, white);
		}
	}


	/*for (int i = 0; i < 1000000; i++) {
		line(13, 20, 80, 40, image, white);
		line(20, 13, 40, 80, image, red);
		line(80, 40, 13, 20, image, red);
	}*/
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}
