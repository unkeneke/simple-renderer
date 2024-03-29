#include <iostream>
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <shlwapi.h>
#include <vector>
#include "gl_util.h"
#include "shaders.h"


const int WIDTH  = 800;
const int HEIGHT = 800;
const int DEPTH = 255;

const std::wstring OUTPUT_TGA_NAME = L"output.tga";

float *zBuffer = new float[WIDTH * HEIGHT];
Model *model = NULL;
TGAImage *diffuseTexture =  new TGAImage();
Vec2i clamp(WIDTH - 1, HEIGHT - 1);

Vec3f eye(1,1, 3);
Vec3f center(0,0,0);
Vec3f up(0,1,0);
Vec3f camera(0,0,1000);
Vec3f lightDirection(0,0,-1);

Matrix viewport = Util::getViewport(WIDTH, HEIGHT, DEPTH);
Matrix modelView = Util::generateModelView(eye, center, up);
Matrix projection = Util::getProjection(camera);


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

Vec3f calculateCameraVertex(Vec3f& vector) {
	// Let's transform the original 3D vector into 4D for homogeneous coordinates
	// projected, scaled, and turn back to 3D
	Matrix vector4D = Matrix::vectorToMatrix(vector);
 
	GouraudShader shader(viewport, projection, modelView);
	
	Vec3f result = Matrix::matrixToVector( viewport * projection * modelView * vector4D );

	return result;
	
	// This is the "flat" calculation method for the 3D vectors on a 2D plane without camera projection
	// scaled to the resolution of the screen or image
	// Since there is no transformation or rotation of any kind, the camera would be fixed on (0, 0, z)
	// float x0 = (vector.x + 1.) * (float)WIDTH / 2.;
	// float y0 = (vector.y + 1.) * (float)HEIGHT / 2.;
	// float z0 = vector.z * (float)DEPTH;
	
	// return Vec3f(x0, y0, z0);
}

void drawTriangleWithZBuffer(Vec3f *triangleVertex, TGAImage* diffuseTexture, Vec3f *uvTextureVertex, float *zbuffer, TGAImage &image, const float intensity, TGAColor color) { 	
	Vec3f triangleVertexProjected[3];
	
	for (int i = 0; i < 3; i++) {
		triangleVertexProjected[i] = calculateCameraVertex(triangleVertex[i]);
	}

	Vec2i* bboxMin = new Vec2i();
	Vec2i* bboxMax = new Vec2i();
	setScreenBoundaries(triangleVertexProjected, bboxMin, bboxMax, image );
	
	Vec3f P;
	
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
			
			if (color == Util::COLOR_BACKGROUND_GRADIENT) {
				Vec3f normalizedPixel = Util::normalizeVector(&P, WIDTH, HEIGHT, WIDTH + HEIGHT, 1);
				image.set(P.x, P.y, TGAColor(255 * normalizedPixel.x, 255 * normalizedPixel.y,   0,   255));
			} else if (color == Util::COLOR_RANDOM) {
				image.set(P.x, P.y, randomColor);
			} else if (color == Util::COLOR_TEXTURE) {
				if (diffuseTexture == nullptr) {
					image.set(P.x, P.y, Util::COLOR_WHITE * intensity);
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
	for (int i=0; i < model->getTotalFaces(); i++) {
		std::vector<std::vector<int>> face = model->getFaceByIndex(i);
		Vec3f triangleVertex[3] = {};
		Vec3f textureCoords[3];
		for (int j=0; j < 3; j++) {
			std::vector<int> faceVertex = face[j];

			Vec3f vertex = model->getVertexByIndex(faceVertex[0]);

			triangleVertex[j] = vertex;
			
			textureCoords[j] =  model->getTextureVertexByIndex(faceVertex[1]);
		}

		if (enableLight) {
			Vec3f normalVector = (triangleVertex[2]-triangleVertex[0])^(triangleVertex[1]-triangleVertex[0]); 
			normalVector.normalize(); 
			float intensity = normalVector * lightDirection; 
			if (intensity > 0) { 
				drawTriangleWithZBuffer(triangleVertex, diffuseTexture, textureCoords, zBuffer, image, intensity, Util::COLOR_TEXTURE); 
			} 
		} else {
			drawTriangleWithZBuffer(triangleVertex, diffuseTexture, textureCoords, zBuffer, image, 1., Util::COLOR_BACKGROUND_GRADIENT);
		} 
	}
}

void drawWireframeObjModel(TGAImage &image) {
	float* wireframeZBuffer = new float[model->getTotalFaces() * 3];
	for (int i=0; i < model->getTotalFaces(); i++) {
		std::vector<std::vector<int>> face = model->getFaceByIndex(i);
		for (int j=0; j < face.size(); j++) {
			std::vector<int> faceVertexOrigin = face[j];
			Vec3f v0 = model->getVertexByIndex(faceVertexOrigin[0]);

			std::vector<int> faceVertexEnd = face[(j+1)%3];
			Vec3f v1 = model->getVertexByIndex(faceVertexEnd[0]);

			// TODO try at creating a z buffer for the wireframe, needs refinement
			// float indexZ = 0.;
			// indexZ += (v0.z + v1.z) / 2;
			// if (wireframeZBuffer[int(i + j * 3)] >= indexZ) {
			// 	continue;
			// }
			// wireframeZBuffer[int(i + j * 3)] = indexZ;
			
			Vec3f r0 = calculateCameraVertex(v0);
			Vec3f r1 = calculateCameraVertex(v1);
			drawLine(r0.x, r0.y, r1.x, r1.y, image, Util::COLOR_WHITE);
		}
	}
	delete[] wireframeZBuffer;
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

	diffuseTexture->read_tga_file("obj/head_diffuse.tga");
	diffuseTexture->flip_vertically();

	
	// drawTriangleExamples(image);
	drawObjModel(image, diffuseTexture, true, false);
	
	
	image.flip_vertically(); // Origin is at the left bottom corner of the image
	char* outputFileName = Util::convertWStringToCharPtr(OUTPUT_TGA_NAME);
	image.write_tga_file(outputFileName);
	// modelDiffuseTexture->write_tga_file(outputFileName);
	
	delete model;
	delete outputFileName;
	delete diffuseTexture;

	openTGAOutput();
	
	return 0;
}

