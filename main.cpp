#include <iostream>
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <shlwapi.h>
#include <vector>


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green   = TGAColor(0, 255,   0,   255);
const TGAColor blue   = TGAColor(0, 0,   255,   255);
const TGAColor purple   = TGAColor(255, 0,   255,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

std::vector<Vec2f> line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	std::vector<Vec2f> linePoints;
	
	bool steep = false; 
	if (std::abs(x0-x1) < std::abs(y0-y1)) {  // if the line is steep, we transpose the image 
		std::swap(x0, y0); 
		std::swap(x1, y1); 
		steep = true; 
	} 
	if (x0 > x1) { // make it left−to−right 
		std::swap(x0, x1); 
		std::swap(y0, y1); 
	} 
	int dx = x1-x0; 
	int dy = y1-y0; 
	int derror2 = std::abs(dy)*2; 
	int error2 = 0; 
	int y = y0; 
	for (int x=x0; x <= x1; x++) { 
		if (steep) {
			linePoints.push_back(Vec2f(y, x));
			image.set(y, x, color); // if transposed, de−transpose 
		} else {
			linePoints.push_back(Vec2f(x, y));
			image.set(x, y, color); 
		} 
		error2 += derror2; 
		if (error2 > dx) { 
			y += (y1 > y0 ? 1 : -1); 
			error2 -= dx*2; 
		} 
	}

	return linePoints;
} 
 
void drawObjModel(TGAImage &image) {
	for (int i=0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j=0; j < 3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j+1)%3]);
			int x0 = (v0.x + 1.) * width/2.;
			int y0 = (v0.y + 1.) * height/2.;
			int x1 = (v1.x + 1.) * width/2.;
			int y1 = (v1.y + 1.) * height/2.;
			line(x0, y0, x1, y1, image, white);
		}
	}
}

std::vector<Vec2f> drawLine(Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
	std::vector<Vec2f> linePoints = line(v1.x, v1.y, v2.x, v2.y, image, color);
	return linePoints;
}

Vec2f calculateTriangleCentroid(Vec2i t0, Vec2i t1, Vec2i t2) {
	float xc = (t0.x + t1.x + t2.x) / 3;//* 0.33333333333;
	float yc = (t0.y + t1.y + t2.y) / 3;//* 0.33333333333;
	return Vec2f(xc, yc);
}

void drawVectorToPoint(std::vector<Vec2f> linePoints, Vec2f point, TGAImage &image, TGAColor color) {
	for (int i = 0; i < linePoints.size(); i++) {
		line(linePoints.at(i).x, linePoints.at(i).y, point.x, point.y, image, color);
	}
}

void drawTriangleByLineSweeping(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
	// sort the vertices, t0, t1, t2 lower−to−upper
	if (t0.y>t1.y) std::swap(t0, t1); 
	if (t0.y>t2.y) std::swap(t0, t2); 
	if (t1.y>t2.y) std::swap(t1, t2);

	int total_height = t2.y-t0.y; 
	for (int y=t0.y; y<=t1.y; y++) { 
		int segment_height = t1.y-t0.y+1; 
		float alpha = (float)(y-t0.y)/total_height; 
		float beta  = (float)(y-t0.y)/segment_height; // be careful with divisions by zero 
		Vec2i A = t0 + (t2-t0)*alpha; 
		Vec2i B = t0 + (t1-t0)*beta; 
		if (A.x>B.x) std::swap(A, B); 
		for (int j=A.x; j<=B.x; j++) { 
			image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
		} 
	}
	// This needs to be refactored into a single call for both loops 
	for (int y=t1.y; y<=t2.y; y++) { 
		int segment_height = t2.y-t1.y+1; 
		float alpha = (float)(y-t0.y)/total_height; 
		float beta  = (float)(y-t1.y)/segment_height; // be careful with divisions by zero 
		Vec2i A = t0 + (t2-t0)*alpha; 
		Vec2i B = t1 + (t2-t1)*beta; 
		if (A.x>B.x) std::swap(A, B); 
		for (int j=A.x; j<=B.x; j++) { 
			image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
		} 
	} 
}

Vec3f barycentric(Vec2i *pts, Vec2i P) {
	// (xA,xB) = (x2,y2) - (x0,y0)
	// (yA,yB) = (x1,y1) - (x0,y0)
	// (zA,zB) = (x0,y0) - (xP,yP) 
	Vec3f u = Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x)^Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y);
	// `pts` and `P` has integer value as coordinates
	// so `abs(u[2])` < 1 means `u[2]` is 0, that means
	// triangle is degenerate, in this case return something with negative coordinates
	if (std::abs(u.z)<1) {
		return Vec3f(-1,1,1);
	}
	return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 
}  
 
void drawTriangleByBarycentricPoint(Vec2i *pts, TGAImage &image, TGAColor color) { 
	Vec2i bboxmin(image.get_width()-1,  image.get_height()-1); 
	Vec2i bboxmax(0, 0); 
	Vec2i clamp(image.get_width()-1, image.get_height()-1); 
	for (int i=0; i<3; i++) { 
		bboxmin.x = std::max<int>(0, std::min<int>(bboxmin.x, pts[i].x));
		bboxmin.y = std::max<int>(0, std::min<int>(bboxmin.y, pts[i].y));

		bboxmax.x = std::min<int>(clamp.x, std::max<int>(bboxmax.x, pts[i].x));
		bboxmax.y = std::min<int>(clamp.y, std::max<int>(bboxmax.y, pts[i].y));
	} 
	Vec2i P; 
	for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
		for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
			Vec3f bc_screen  = barycentric(pts, P); 
			if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) {
				continue;
			}
			image.set(P.x, P.y, color); 
		} 
	} 
} 

void drawTriangle(Vec2i* triangle, TGAImage &image, TGAColor color) { 
	// drawTriangleByLineSweeping(triangle[0], triangle[1], triangle[2], image, color);
	drawTriangleByBarycentricPoint(triangle, image, color);
}

Vec2i scaleVector(Vec2i vector) {
	return vector * 4;
}

void drawTriangles(TGAImage &image) {
	Vec2i t0[3] = { scaleVector(Vec2i(10, 70)),   scaleVector(Vec2i(50, 160)),  scaleVector(Vec2i(70, 80)) }; 
	Vec2i t1[3] = { scaleVector(Vec2i(180, 50)),  scaleVector(Vec2i(150, 1)),   scaleVector(Vec2i(70, 180)) }; 
	Vec2i t2[3] = { scaleVector(Vec2i(180, 150)), scaleVector(Vec2i(120, 160)), scaleVector(Vec2i(130, 180)) }; 
	drawTriangle(t0, image, red); 
	drawTriangle(t1, image, white); 
	drawTriangle(t2, image, green);
}

void openTGAOutput() {
	SHELLEXECUTEINFOW ShExecInfo = {};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
	ShExecInfo.lpVerb = L"edit";
	ShExecInfo.lpFile = L"D:/_DEV/tinyrenderer/simplerenderer/output.tga";
	ShExecInfo.nShow = SW_MAXIMIZE;
	ShExecInfo.fMask = SEE_MASK_NOASYNC;

	ShellExecuteEx(&ShExecInfo);

	if (!ShellExecuteExW(&ShExecInfo)) {
		// Error reported in GetLastError()
	}
}

int main(int argc, char** argv) {
	if (2 == argc) {
		model = new Model(argv[1]);
	} else {
		model = new Model("obj/head.obj");
	}
	
	TGAImage image(width, height, TGAImage::RGB);
	
	
	drawTriangles(image);
	
	
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;

	openTGAOutput();
	
	return 0;
}

