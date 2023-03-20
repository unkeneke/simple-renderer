#include <iostream>
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <shlwapi.h>
#include <vector>
#include "util.h"


const TGAColor COLOR_WHITE = TGAColor(255, 255, 255, 255);
const TGAColor COLOR_RED   = TGAColor(255, 0,   0,   255);
const TGAColor COLOR_GREEN   = TGAColor(0, 255,   0,   255);
const TGAColor COLOR_BLUE   = TGAColor(0, 0,   255,   255);
const TGAColor COLOR_PURPLE   = TGAColor(255, 0,   255,   255);
const TGAColor COLOR_BACKGROUND_GRADIENT = TGAColor(-1, 0,   0,   255);
const TGAColor COLOR_RANDOM = TGAColor(-2, 0,   0,   255);
const TGAColor COLOR_TEXTURE = TGAColor(-3, 0,   0,   255);

const int WIDTH  = 800;
const int HEIGHT = 800;

const std::wstring OUTPUT_TGA_NAME = L"output.tga";

float *zBuffer = new float[WIDTH * HEIGHT];
Model *model = NULL;
TGAImage *modelDiffuseTexture =  new TGAImage();
Vec3f lightDirection(0,0,-1);
Vec2i clamp(WIDTH - 1, HEIGHT - 1); 

std::vector<Vec2f> drawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
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

Vec2f calculateTriangleCentroid(Vec2i t0, Vec2i t1, Vec2i t2) {
	float xc = (t0.x + t1.x + t2.x) / 3;//* 0.33333333333; // this could be faster than division
	float yc = (t0.y + t1.y + t2.y) / 3;//* 0.33333333333;
	return Vec2f(xc, yc);
}

void drawVectorToPoint(std::vector<Vec2f> linePoints, Vec2f point, TGAImage &image, TGAColor color) {
	for (int i = 0; i < linePoints.size(); i++) {
		drawLine(linePoints.at(i).x, linePoints.at(i).y, point.x, point.y, image, color);
	}
}

void drawTriangleByLineSweeping(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
	// sort the vertices, t0, t1, t2 lower−to−upper
	if (t0.y>t1.y) std::swap(t0, t1); 
	if (t0.y>t2.y) std::swap(t0, t2); 
	if (t1.y>t2.y) std::swap(t1, t2);

	int totalHeight = t2.y - t0.y;
	
	for (int i = 0; i < totalHeight; i++) { 
		bool secondHalf = i>t1.y-t0.y || t1.y==t0.y; 
		int segmentHeight = secondHalf ? t2.y-t1.y : t1.y-t0.y; 
		float alpha = (float)i/totalHeight; 
		float beta = (float)(i-(secondHalf ? t1.y-t0.y : 0)) / segmentHeight;
		Vec2i A = t0 + (t2-t0) * alpha; 
		Vec2i B = secondHalf ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta; 
		if (A.x>B.x) {
			std::swap(A, B);
		}
		for (int j=A.x; j<=B.x; j++) { 
			image.set(j, t0.y+i, color);
		} 
	} 
}

void drawTriangle(Vec2i* triangle, TGAImage &image, TGAColor color) { 
	drawTriangleByLineSweeping(triangle[0], triangle[1], triangle[2], image, color);
}

Vec2i scaleVector(Vec2i vector) {
	return vector * 4;
}

void drawTriangleExamples(TGAImage &image) {
	Vec2i t0[3] = { scaleVector(Vec2i(10, 70)),   scaleVector(Vec2i(50, 160)),  scaleVector(Vec2i(70, 80)) }; 
	Vec2i t1[3] = { scaleVector(Vec2i(180, 50)),  scaleVector(Vec2i(150, 1)),   scaleVector(Vec2i(70, 180)) }; 
	Vec2i t2[3] = { scaleVector(Vec2i(180, 150)), scaleVector(Vec2i(120, 160)), scaleVector(Vec2i(130, 180)) };
	drawTriangle(t0, image, COLOR_RED); 
	drawTriangle(t1, image, COLOR_GREEN); 
	drawTriangle(t2, image, COLOR_WHITE);
}

void rasterize2dDepthBuffer(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]) {
	if (p0.x>p1.x) {
		std::swap(p0, p1);
	}
	for (int x=p0.x; x<=p1.x; x++) {
		float t = (x-p0.x)/(float)(p1.x-p0.x);
		int y = p0.y*(1.-t) + p1.y*t;
		if (yBuffer[x]<y) {
			yBuffer[x] = y;
			image.set(x, 0, color);
		}
	}
}

Vec3f getBarycentricVector(Vec3f *triangleVertex, Vec3f P) {
	// This calculation comes from a linear system of equations when considering u + v + w = 1 in barycentric coordinate theory
	// The result is a vector [u, v, 1] that is perpendicular to (ACx, ABx, PAx) and (ACy, ABy, PAy)
	// (ACx, ABx, PAx) cross product (ACy, ABy, PAy) should give us the normal vector, with a z value that must be 1
	// if not, P doesn't belong in this triangle 
	Vec3f barycentricWeight = Vec3f(triangleVertex[2].x-triangleVertex[0].x, triangleVertex[1].x-triangleVertex[0].x, triangleVertex[0].x-P.x)^Vec3f(triangleVertex[2].y-triangleVertex[0].y, triangleVertex[1].y-triangleVertex[0].y, triangleVertex[0].y-P.y);

	// triangleVertex and P has integer value as coordinates
	// so abs(barycentricWeight[2]) < 1 means barycentricWeight[2] is 0, that means
	// triangle is degenerate, in this case return something with negative coordinates
	if (std::abs(barycentricWeight.z)<1) {
		return Vec3f(-1,1,1);
	}
	return Vec3f(1.f-(barycentricWeight.x+barycentricWeight.y)/barycentricWeight.z, barycentricWeight.y/barycentricWeight.z, barycentricWeight.x/barycentricWeight.z); 
}

void drawWireframeObjModel(TGAImage &image) {
	float* wireframeZBuffer = new float[model->totalFaces() * 3];
	for (int i=0; i < model->totalFaces(); i++) {
		std::vector<std::vector<int>> face = model->getFaceByIndex(i);
		for (int j=0; j < face.size(); j++) {
			std::vector<int> faceVertexOrigin = face[j];
			Vec3f v0 = model->getVertexByIndex(faceVertexOrigin[0]);

			std::vector<int> faceVertexEnd = face[(j+1)%3];
			Vec3f v1 = model->getVertexByIndex(faceVertexEnd[0]);
		
			
			int x0 = (v0.x + 1.) * WIDTH/2.;
			int y0 = (v0.y + 1.) * HEIGHT/2.;
			int x1 = (v1.x + 1.) * WIDTH/2.;
			int y1 = (v1.y + 1.) * HEIGHT/2.;

			// TODO try at creating a z buffer for the wireframe, needs refinement
			// float indexZ = 0.;
			// indexZ += (v0.z + v1.z) / 2;
			// if (wireframeZBuffer[int(i + j * 3)] >= indexZ) {
			// 	continue;
			// }
			// wireframeZBuffer[int(i + j * 3)] = indexZ;
			
			drawLine(x0, y0, x1, y1, image, COLOR_WHITE);
		}
	}
	delete wireframeZBuffer;
}

void setScreenBoundaries(Vec3f *triangleVertex, Vec2i* bboxMin, Vec2i* bboxMax, TGAImage &image) {
	bboxMin->u = image.get_width()-1;
	bboxMin->v =  image.get_height()-1; 
	bboxMax->u = 0;
	bboxMax->v = 0;
	for (int i=0; i<3; i++) { 
		bboxMin->x = std::max<int>(0, std::min<int>(bboxMin->x, triangleVertex[i].x));
		bboxMin->y = std::max<int>(0, std::min<int>(bboxMin->y, triangleVertex[i].y));

		bboxMax->x = std::min<int>(clamp.x, std::max<int>(bboxMax->x, triangleVertex[i].x));
		bboxMax->y = std::min<int>(clamp.y, std::max<int>(bboxMax->y, triangleVertex[i].y));
	} 
}
Vec3f* calculateZPerspective(Vec3f* vector, float zConstant) {
	Vec4f vector4D(vector->x,vector->y,vector->z,1);

	Vec4f matrix[4] = {
		Vec4f(1., 0., 0., 0.),
		Vec4f(0., 1., 0., 0.),
		Vec4f(0., 0., 1., 0.),
		Vec4f(0., 0., -1./zConstant, 1.)
	};

	float values[4];
	for (int i = 0; i < 5; i++) {
		values[i] = matrix[i] * vector4D;
	}

	Vec4f result4D(values[0],values[1],values[2],values[3]);
	Vec3f* result = result4D.projectTo3D();
	// std::cout << result;
	return result;
}

void drawTriangleWithZBuffer(Vec3f *triangleVertex, Vec3f *originaVertex, TGAImage* diffuseTexture, Vec3f *uvTextureVertex, float *zbuffer, TGAImage &image, const float intensity, TGAColor color) { 	
	Vec2i* bboxMin = new Vec2i();
	Vec2i* bboxMax = new Vec2i();
	setScreenBoundaries(triangleVertex, bboxMin, bboxMax, image );
	
	Vec3f P;
	Vec3f* triangleVertexProjected = new Vec3f[3];

	for (int i = 0; i < 4; i++) {
		triangleVertexProjected[i] = *calculateZPerspective(&triangleVertex[i], -1./5.);
	}
	
	TGAColor randomColor(rand() % 255, rand() % 255, rand() % 255, 255);

	for (P.x = bboxMin->x; P.x <= bboxMax->x; P.x++) { 
		for (P.y = bboxMin->y; P.y <= bboxMax->y; P.y++) {
			Vec3f barycentricWeights  = getBarycentricVector(triangleVertexProjected, P); 
			if (barycentricWeights.x < 0 || barycentricWeights.y < 0 || barycentricWeights.z < 0) {
				// Barycentric point is out of the triangle's area, so not a valid coordinate
				continue;
			}
			
			P.z = 0;
			P.z += triangleVertexProjected[0].z * barycentricWeights.x;
			P.z += triangleVertexProjected[1].z * barycentricWeights.y;
			P.z += triangleVertexProjected[2].z * barycentricWeights.z;
			if (zbuffer[int(P.x + P.y * WIDTH)] >= P.z) {
				continue;
			}

			// This is a visible point, update the Z Buffer
			zbuffer[int(P.x + P.y * WIDTH)] = P.z;
			
			if (color == COLOR_BACKGROUND_GRADIENT) {
				Vec3f normalizedPixel = Util::normalizeVector(&P, WIDTH, HEIGHT, WIDTH + HEIGHT, 1);
				image.set(P.x, P.y, TGAColor(255 * normalizedPixel.x, 255 * normalizedPixel.y,   0,   255));
			} else if (color == COLOR_RANDOM) {
				image.set(P.x, P.y, randomColor);
			} else if (color == COLOR_TEXTURE) {
				if (diffuseTexture == nullptr) {
					image.set(P.x, P.y, COLOR_WHITE * intensity);
					continue;
				}

				// We use the calculated barycentricWeights from P across the original triangle
				// And interpolate it through the texture triangle
				Vec3f interpolatedPoint = uvTextureVertex[0] * barycentricWeights.x + uvTextureVertex[1] * barycentricWeights.y + uvTextureVertex[2] * barycentricWeights.z;
					
				TGAColor sectionColor = diffuseTexture->get(
					(float)diffuseTexture->get_width() * interpolatedPoint.x,
					(float)diffuseTexture->get_height() * interpolatedPoint.y
				);
					
				image.set(P.x, P.y, sectionColor * intensity);
			} else {
				image.set(P.x, P.y, color * intensity);
			}
		} 
	}

	delete bboxMax;
	delete bboxMin;
}

void drawTriangleSurfaces(TGAImage &image, TGAImage* diffuseTexture, bool enableLight) {
	for (int i=0; i < model->totalFaces(); i++) {
		std::vector<std::vector<int>> face = model->getFaceByIndex(i);
		Vec3f triangleVertex[3] = {};
		Vec3f originaVertex[3] = {};
		Vec3f worldCoords[3];
		Vec3f textureCoords[3];
		for (int j=0; j < 3; j++) {
			std::vector<int> faceVertex = face[j];

			Vec3f vertex = model->getVertexByIndex(faceVertex[0]);

			// Scaling the x and y of each vertex to the size of the screen/image
			int x0 = (vertex.x + 1.) * WIDTH / 2.;
			int y0 = (vertex.y + 1.) * HEIGHT / 2.;
			int z0 = 0;

			triangleVertex[j] = Vec3f(x0, y0, z0);
			originaVertex[j] = vertex;

			worldCoords[j]  = vertex;
			
			textureCoords[j] =  model->getTextureVertexByIndex(faceVertex[1]);
		}

		if (enableLight) {
			Vec3f normalVector = (worldCoords[2]-worldCoords[0])^(worldCoords[1]-worldCoords[0]); 
			normalVector.normalize(); 
			float intensity = normalVector * lightDirection; 
			if (intensity > 0) { 
				drawTriangleWithZBuffer(triangleVertex, originaVertex, diffuseTexture, textureCoords, zBuffer, image, intensity, COLOR_TEXTURE); 
			} 
		} else {
			drawTriangleWithZBuffer(triangleVertex, originaVertex, diffuseTexture, textureCoords, zBuffer, image, 1., COLOR_BACKGROUND_GRADIENT);
		} 
	}
}

void drawObjModel(TGAImage &image, TGAImage* diffuseTexture, bool enableLight, bool enableWireframe) {
	if (diffuseTexture != nullptr) {
		drawTriangleSurfaces(image, diffuseTexture, enableLight);
	}
	
	if (enableWireframe) {
		drawWireframeObjModel(image);
	} 
}

void openTGAOutput() {
	SHELLEXECUTEINFOW ShExecInfo = {};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
	ShExecInfo.lpVerb = L"edit";
	ShExecInfo.lpFile = OUTPUT_TGA_NAME.c_str();
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
	
	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

	modelDiffuseTexture->read_tga_file("obj/head_diffuse.tga");
	modelDiffuseTexture->flip_vertically();

	
	// drawTriangleExamples(image);
	drawObjModel(image, modelDiffuseTexture, true, false);
	
	
	image.flip_vertically(); // Origin is at the left bottom corner of the image
	char* outputFileName = Util::convertWStringToCharPtr(OUTPUT_TGA_NAME);
	image.write_tga_file(outputFileName);
	// modelDiffuseTexture->write_tga_file(outputFileName);
	
	delete model;
	delete outputFileName;
	delete modelDiffuseTexture;

	openTGAOutput();
	
	return 0;
}

