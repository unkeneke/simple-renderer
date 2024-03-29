 #ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>


template <class t> struct Vec2 {
	union {
		struct {t u, v;};
		struct {t x, y;};
		t raw[2];
	};
	Vec2() : u(0), v(0) {}
	Vec2(t _u, t _v) : u(_u),v(_v) {}
	Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(u+V.u, v+V.v); }
	Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(u-V.u, v-V.v); }
	Vec2<t> operator *(float f)          const { return Vec2<t>(u*f, v*f); }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;

template <class t> struct Vec3 {
	union {
		struct {t x, y, z;};
		struct { t ivert, iuv, inorm; };
		t raw[3];
	};
	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x),y(_y),z(_z) {}
	Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
	Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
	Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
	Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
	t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
	t&      operator [](const int i) { if (i<=0) return x; else if (i==1) return y; else return z; }
	
	float norm () const { return std::sqrt(x*x+y*y+z*z); }
	Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

template <class t> struct Vec4 {
	union {
		struct {t x, y, z, w;};
		t raw[4];
	};
	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(t _x, t _y, t _z, t _w) : x(_x), y(_y), z(_z), w(_w) {}
	inline t       operator *(const Vec4<t> &v) const { return x*v.x + y*v.y + z*v.z + w*v.w; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec4<t>& v);
	
	Vec3f projectTo3D() {
		Vec3f result(x/w, y/w, z/w);
		return result;
	}
};

typedef Vec4<float> Vec4f;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}

const int DEFAULT_ALLOC=4;

class Matrix {
	std::vector<std::vector<float> > m;
	int rows, cols;
public:
	Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC);
	inline int getTotalRows();
	inline int getTotalColumns();

	static Matrix identity(int dimensions);
	std::vector<float>& operator[](const int i);
	Matrix operator*(const Matrix& a);
	Matrix transpose();
	Matrix inverse();
	static Matrix vectorToMatrix(Vec3f v);
	static Vec3f matrixToVector(Matrix m);

	friend std::ostream& operator<<(std::ostream& s, Matrix& m);
};

#endif //__GEOMETRY_H__
