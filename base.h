#pragma once
#include<vector>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
const int width = 800;
const int height = 500;
const int depth = 255;
extern std::vector<int> zbuffer;
extern std::shared_ptr<Model> model;

extern Vec3f lightDir;
extern Vec3f camera;
extern Vec3f eye;
extern Vec3f center;

Matrix viewport(int x, int y, int w, int h);
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);
 
extern Matrix ModelView;
extern Matrix Projection;
extern Matrix ViewPort;

extern TGAImage zbufferImage;

class IShader 
{
public:
	virtual ~IShader() {}
	virtual Vec3i vert(int face_id, int vertex_id) = 0;
	virtual bool frag(Vec3f bar, TGAColor& color) = 0;
};

class GouraudShader :public IShader
{
public:
	float intensity;
	Vec2i uv_coords[3];
	Vec2i uv_coord;
	virtual ~GouraudShader() {};
	virtual Vec3i vert(int face_id, int vertex_id);
	virtual bool frag(Vec3f bar, TGAColor& color);
};


Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P);
void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color);
void triangle(Vec3i* t, float* ity, TGAImage& image);
void triangle_frag(Vec3i* t, Vec2i* uv,TGAImage& tex, TGAImage& image);
void rasterization(Vec3i* t, IShader& shader, TGAImage& image, TGAImage& zbuffer);