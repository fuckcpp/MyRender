#pragma once
#include <vector>
#include <memory>
#include "geometry.h"
#include "tgaimage.h"
#include "base.h"

class Model
{
public:
	Model(const char* filename);
	~Model();

	void setTexture(std::unique_ptr<TGAImage> tex);
	TGAImage& getTexture();
	int nverts();
	int nfaces();
	int nvt_faces();
	int nvts();
	Vec3f vert(int i);
	Vec3f uv(int i,int j);
	Vec3f norm(int i, int j);
	std::vector<int> face(int idx);
	void draw(TGAImage& image);
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> vts_;
	std::vector<Vec3f> vns_;
	std::vector<std::vector<int>> faces_;
	std::vector<std::vector<int>> vt_faces_;
	std::vector<std::vector<int>> vn_faces_;
	std::unique_ptr<TGAImage> texture;
};