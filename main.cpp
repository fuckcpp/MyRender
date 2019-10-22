#include <iostream>
#include <memory>
#include "tgaimage.h"
#include "Model.h"
#include "base.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
extern std::vector<int> zbuffer;
int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
	std::shared_ptr<Model> model = std::make_shared<Model>("obj/african_head.obj");
	std::unique_ptr<TGAImage> texture(new TGAImage());
	texture->read_tga_file("texture/african_head_diffuse.tga");
	model->setTexture(std::move(texture));
	model->draw(image);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	TGAImage zbufferImage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			zbufferImage.set(i,j, TGAColor(zbuffer[i+width*j],1));
	}
	zbufferImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbufferImage.write_tga_file("zbuffer.tga");
	return 0;
}
