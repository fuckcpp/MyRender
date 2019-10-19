#pragma once
#include <vector>
#include <memory>
#include "geometry.h"
#include "tgaimage.h"
class Model
{
public:
	Model(const char* filename);
	~Model();

	bool setTexture(std::weak_ptr<TGAImage> tex);
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f uv(int i);
	std::vector<int> face(int idx);

private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> vts_;
	std::vector<std::vector<int>> faces_;
	std::vector<std::vector<int>> vt_faces_;
	std::shared_ptr<TGAImage> texture;
};