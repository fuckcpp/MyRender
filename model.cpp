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
		}else if(!line.compare(0, 2, "f "))
		{
			//f 1215 / 1270 / 1215 1258 / 1339 / 1258 1217 / 1275 / 1217
			iss >> trash;
			std::vector<int> f;
			int itrash, idex;
			while (iss>>idex>>trash>>itrash>>trash>>itrash)
			{
				idex--;
				f.push_back(idex);
			}
			faces_.push_back(f);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f#" << faces_.size() << std::endl;
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

Vec3f Model::Vert(int i)
{
	return verts_[i];
}

std::vector<int> Model::face(int idx)
{
	return faces_[idx];
}
