#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
Model::Model(const char* filename)
{
	std::ifstream in;
	in.open(filename,std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof())
	{
		std::getline(in,line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v "))
		{
			iss >> trash;//v 字符放到了这里
			Vec3f v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 4, "vt  "))
		{
			iss >> trash >> trash;//vt 字符放到了这里
			Vec3f vt;
			for (int i = 0; i < 3; i++) iss >> vt[i];
			vts_.push_back(vt);
		}
		else if(!line.compare(0, 2, "f "))
		{
			iss >> trash;
			std::vector<int> f;
			std::vector<int> vt_f;
 			int itrash, v_idex, vt_idx;
			while (iss>> v_idex >>trash>> vt_idx >>trash>>itrash)
			{
				f.push_back(--v_idex);
				vt_f.push_back(--vt_idx);
			}
			faces_.push_back(f);
			vt_faces_.push_back(vt_f);
		}
	}
	std::cerr << "# v# " << verts_.size() 
		<< " vt#" << vts_.size()
		<< " f#" << faces_.size()
		<< " vt_f#" << vt_faces_.size() << std::endl;

	in.close();

}

void Model::setTexture(std::unique_ptr<TGAImage> tex)
{
	texture = std::move(tex);
}

TGAImage& Model::getTexture()
{
	return *texture;
}
Model::~Model()
{
}

int Model::nverts()
{
	return verts_.size();
}

int Model::nfaces()
{
	return faces_.size();
}

int Model::nvt_faces()
{
	return vt_faces_.size();
}

int Model::nvts()
{
	return vts_.size();
}

Vec3f Model::vert(int i)
{
	if (i < verts_.size())
		return verts_[i];
	else
		return Vec3f();
}

Vec3f Model::uv(int i)
{
	if (i < vts_.size())
		return vts_[i];
	else
		return Vec3f();
}

std::vector<int> Model::face(int idx)
{
	return faces_[idx];
}

std::vector<int> Model::vt_face(int idx)
{
	return vt_faces_[idx];
}

void Model::draw(TGAImage& image)
{
	Vec3f lightDir = { 0,0,-1 };
	for (int i = 0; i < nfaces(); i++)
	{
		std::vector<int> face = this->face(i);
		Vec3i screen_coords[3];
		Vec3f world_pos[3];
		float intensity;
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = vert(face[j]);
			screen_coords[j].x = (v.x + 1.) * width / 2;
			screen_coords[j].y = (v.y + 1.) * height / 2;
			screen_coords[j].z = (v.z + 1.) * depth / 2;
			world_pos[j] = v;
		}
		//光照计算
		Vec3f normal = (world_pos[2] - world_pos[0]) ^ (world_pos[1] - world_pos[0]);
		intensity = normal.normlize() * lightDir;

		Vec2i uv_coords[3];
		if (nvt_faces() && nvts())
		{
			std::vector<int> vt_face = this->vt_face(i);
			for (int j = 0; j < 3; j++)
			{
				Vec3f uv = this->uv(vt_face[j]);
				uv_coords[j].x = (1. - uv.x) * getTexture().get_width();
				uv_coords[j].y = (1. - uv.y) * getTexture().get_height();
			}
			if (intensity > 0)//back_culling 剔除反面的三角形
			{
				triangle_frag(screen_coords, uv_coords, getTexture(), intensity, image);
			}
		}
		else
		{
			if (intensity > 0)//back_culling 剔除反面的三角形
			{
				triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
			}

		}

	}
}