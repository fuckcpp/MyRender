#include "base.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

extern std::vector<int> zbuffer(width* height, std::numeric_limits<int>::min());
extern std::shared_ptr<Model> model = std::make_shared<Model>("obj/african_head.obj");
extern Vec3f lightDir = Vec3f(1,-1,1).normalize();
extern Vec3f camera = { 0,0,3 };
extern Vec3f eye(1, 1, 3);
extern Vec3f center(0, 0, 0);

extern Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
extern Matrix Projection = Matrix::identity(4);
extern Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
//extern Matrix ViewPort = viewport(0, 0, width , height);

extern TGAImage zbufferImage=TGAImage(width, height, TGAImage::GRAYSCALE);


//裁剪空间到屏幕空间的转换矩阵
Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();
	Matrix res = Matrix::identity(4);
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[3][i] = -center[i];
	}
	return res;
}

Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P) {
	Vec3f u = Vec3f(C.x - A.x, B.x - A.x, A.x - P.x) ^ Vec3f(C.y - A.y, B.y - A.y, A.y - P.y);
	return std::abs(u.z) > .5 ? Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z) : Vec3f(-1, 1, 1); // dont forget that u.z is an integer. If it is zero then triangle ABC is degenerate
}



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

void triangle(Vec3i* t, float* ity, TGAImage& image)
{
	if (t[0].y == t[1].y && t[0].y == t[2].y) return; // i dont care about degenerate triangles
	if (t[0].y > t[1].y) { std::swap(t[0], t[1]); std::swap(ity[0], ity[1]); }
	if (t[0].y > t[2].y) { std::swap(t[0], t[2]); std::swap(ity[0], ity[2]); }
	if (t[1].y > t[2].y) { std::swap(t[1], t[2]); std::swap(ity[1], ity[2]); }
	int total_height = t[2].y - t[0].y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t[1].y - t[0].y || t[1].y == t[0].y;
		int seg_height = second_half ? t[2].y - t[1].y : t[1].y - t[0].y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t[1].y - t[0].y : 0)) / seg_height; // be careful: with above conditions no division by zero here
		Vec3i A = t[0] + Vec3f(t[2] - t[0]) * alpha;
		Vec3i B = second_half ? t[1] + Vec3f(t[2] - t[1]) * beta : t[0] + Vec3f(t[1] - t[0]) * beta;
		float ityA = ity[0] + (ity[2] - ity[0]) * alpha;
		float ityB = second_half ? ity[1] + (ity[2] - ity[1]) * beta : ity[0] + (ity[1] - ity[0]) * beta;
		if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); }
		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (j - A.x) / (float)(B.x - A.x);
			Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
			float ityP = ityA + (ityB - ityA) * phi;
			int index = P.y * width + P.x;
			if (0 <= index && index < width * height)//防止顶点坐标超出视口范围，导致深度缓冲越界
			{
				if (zbuffer[index] < P.z)//必须做比较
				{
					zbuffer[index] = P.z;
					image.set(P.x, P.y, TGAColor(255 * ityP, 255 * ityP, 255 * ityP, 255)); // attention, due to int casts t[0].y+i != A.y
				}
			}
		}

	}
}

void triangle_frag(Vec3i* t, Vec2i* uv, TGAImage& tex, TGAImage& image)
{
	if (t[0].y == t[1].y && t[0].y == t[2].y) return; // i dont care about degenerate triangles
	if (t[0].y > t[1].y) { std::swap(t[0], t[1]); std::swap(uv[0], uv[1]);}
	if (t[0].y > t[2].y) { std::swap(t[0], t[2]); std::swap(uv[0], uv[2]);}
	if (t[1].y > t[2].y) { std::swap(t[1], t[2]); std::swap(uv[1], uv[2]);}
	int total_height = t[2].y - t[0].y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t[1].y - t[0].y || t[1].y == t[0].y;
		int seg_height = second_half ? t[2].y - t[1].y : t[1].y - t[0].y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t[1].y - t[0].y : 0)) / seg_height; // be careful: with above conditions no division by zero here
		Vec3i A = t[0] + Vec3f(t[2] - t[0]) * alpha;
		Vec3i B = second_half ? t[1] + Vec3f(t[2] - t[1]) * beta : t[0] + Vec3f(t[1] - t[0]) * beta;
		Vec2i uvA = uv[0] + (uv[2] - uv[0]) * alpha;
		Vec2i uvB = second_half ? uv[1] + (uv[2] - uv[1]) * beta : uv[0] + (uv[1] - uv[0]) * beta;
		if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB);}
		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (j - A.x) / (float)(B.x - A.x);
			Vec2i UV= uvA + (uvB - uvA) * phi;;
			Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
			TGAColor color = tex.get(UV.x, UV.y);
			int index = P.y * width + P.x;
			if (0 <= index && index < width * height)//防止顶点坐标超出视口范围，导致深度缓冲越界
			{
				if (zbuffer[index] < P.z)//必须做比较
				{
					zbuffer[index] = P.z;
					image.set(P.x, P.y, color); // attention, due to int casts t[0].y+i != A.y
				}
			}
		}
	}
}

void rasterization(Vec3i* t, IShader& shader, TGAImage& image, TGAImage& zbuffer)
{
	Vec2i minBox(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	Vec2i maxBox(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			minBox[j] = std::min(minBox[j], t[i][j]);
			maxBox[j] = std::max(maxBox[j], t[i][j]);
		}
	}
	Vec3i Itr;
	for(int x = minBox.x; x < maxBox.x; x++)
	{
		for (int y = minBox.y; y < maxBox.y; y++)
		{
			Itr.x = x;
			Itr.y = y;
			Vec3f bary = barycentric(t[0], t[1], t[2], Itr);//遍历三角形所在矩形，计算每个像素的重心坐标
			//使用重心坐标进行插值计算
			Itr.z = std::max(0, std::min(255,int(t[0].z*bary.x+ t[1].z * bary.y+ t[2].z * bary.z+.5)));
			TGAColor color;
			if (bary.x < 0 || bary.y < 0 || bary.z < 0 || Itr.z < zbuffer.get(x,y)[0]) continue;
			if (!shader.frag(bary, color))
			{
				image.set(Itr.x, Itr.y, color);
				zbuffer.set(Itr.x, Itr.y, TGAColor(Itr.z));
			}
		}
	}
}

Vec3i GouraudShader::vert(int face_id, int vertex_id)
{
	std::vector<int> face = model->face(face_id);
	Vec3f v = model->vert(face[vertex_id]);
	Vec3i screen_coord = Vec3f(ViewPort * Projection * ModelView * Matrix(v));
	intensity = model->norm(face_id, vertex_id) * lightDir;
	Vec3f uv = model->uv(face_id, vertex_id);
	uv_coord.x = uv.x * model->getTexture().get_width();
	uv_coord.y = (1. - uv.y) * model->getTexture().get_height();
	return screen_coord;
}

bool GouraudShader::frag(Vec3f bar, TGAColor& color)
{
	Vec2i uv = uv_coords[0] * bar[0] + uv_coords[1] * bar[1] + uv_coords[2] * bar[2];
	color = model->getTexture().get(uv.x, uv.y);
	return false;
}
