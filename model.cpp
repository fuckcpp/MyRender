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
			for (int i = 0; i < 3; i++) iss >> v.raw[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 4, "vt  "))
		{
			iss >> trash >> trash;//v 字符放到了这里
			Vec3f vt;
			for (int i = 0; i < 3; i++) iss >> vt.raw[i];
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
}

bool Model::setTexture(std::weak_ptr<TGAImage> tex)
{
	if (tex.expired())
	{
		texture = tex.lock();
		return true;
	}
	return false;
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
