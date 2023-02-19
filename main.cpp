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

std::vector<Vec2f> drawScaledLine(Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color, int sectionSize) {
	std::vector<Vec2f> linePoints = line(
		(width * v1.x) / sectionSize,
		(height * v1.y) / sectionSize,
		(width * v2.x) / sectionSize,
		(height * v2.y) / sectionSize,
		image,
		color
	);

	return linePoints;
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

void drawScaledTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color, int sectionSize) { 
	std::vector<Vec2f> line1Points = drawScaledLine(t0, t1, image, color, sectionSize); 
	std::vector<Vec2f> line2Points = drawScaledLine(t1, t2, image, color, sectionSize); 
	std::vector<Vec2f> line3Points = drawScaledLine(t2, t0, image, color, sectionSize);

	Vec2f centroid = calculateTriangleCentroid(t0, t1, t2);
	Vec2f scaledCentroid = Vec2f((width * centroid.x) / sectionSize,(height * centroid.y) / sectionSize);

	drawVectorToPoint(line1Points, scaledCentroid, image, color);
	drawVectorToPoint(line2Points, scaledCentroid, image, color);
	drawVectorToPoint(line3Points, scaledCentroid, image, color);
}

void drawTriangles(TGAImage &image) {
	int sectionSize = 200;
	Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
	Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
	Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
	drawScaledTriangle(t0[0], t0[1], t0[2], image, red, sectionSize); 
	drawScaledTriangle(t1[0], t1[1], t1[2], image, white, sectionSize); 
	drawScaledTriangle(t2[0], t2[1], t2[2], image, green, sectionSize);
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
		model = new Model("obj/african_head.obj");
	}
	
	TGAImage image(width, height, TGAImage::RGB);
	
	
	drawTriangles(image);
	
	
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;

	openTGAOutput();
	
	return 0;
}

