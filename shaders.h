#ifndef __SHADERS_H__
#define __SHADERS_H__

#include "geometry.h"
#include "tgaimage.h"

class IShader {
protected:
	Matrix* viewport;
	Matrix* projection;
	Matrix* modelView;
	
public:
	IShader(Matrix& viewport, Matrix& projection, Matrix& modelView) {
		this->viewport = &viewport;
		this->projection = &projection;
		this->modelView = &modelView;
	}
	virtual ~IShader() {
		// needs testing for children classes
	}
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

class GouraudShader : public IShader {
private:
	Vec3f varying_intensity; // written by vertex shader, read by fragment shader

public:
	GouraudShader(Matrix& viewport, Matrix& projection, Matrix& modelView) : IShader(viewport, projection, modelView) {
		
	}
	Vec4f vertex(int iface, int nthvert) override;
	bool fragment(Vec3f bar, TGAColor &color) override;
};

#endif //__SHADERS_H__
