#ifndef MAT_H
#define MAT_H
#include <array>

// #define MATH_FORCE_DEPTH_ZERO_TO_ONE

namespace maths {
class Vec3;

class Mat4 {
	typedef float						InternalType;
	typedef std::array<InternalType, 4> Line;
	typedef std::array<Line, 4>			Repr;

public:
	static Mat4 identity;

	explicit	Mat4(const Line &l1, const Line &l2, const Line &l3, const Line &l4);

	// clang-format off
	explicit	Mat4(InternalType c11 = 0, InternalType c12 = 0, InternalType c13 = 0, InternalType c14 = 0,
					 InternalType c21 = 0, InternalType c22 = 0, InternalType c23 = 0, InternalType c24 = 0,
					 InternalType c31 = 0, InternalType c32 = 0, InternalType c33 = 0, InternalType c34 = 0,
					 InternalType c41 = 0, InternalType c42 = 0, InternalType c43 = 0, InternalType c44 = 0);
	// clang-format on

	const Line &operator[](size_t idx) const;
	Line	   &operator[](size_t idx);

	Mat4		operator+(const Mat4 &other) const;
	Mat4	   &operator+=(const Mat4 &other);

	Mat4		operator-(const Mat4 &other) const;
	Mat4	   &operator-=(const Mat4 &other);

	Mat4		operator*(const Mat4 &other) const;
	Mat4	   &operator*=(const Mat4 &other);
	Mat4		operator*(InternalType lambda) const;
	Mat4	   &operator*=(InternalType lambda);

	Mat4		operator/(InternalType lambda) const;
	Mat4	   &operator/=(InternalType lambda);

	Mat4		operator-() const;

	static Mat4 rotate(InternalType angle, const Vec3 &u);
	static Mat4 lookAt(const Vec3 &eye, const Vec3& center, const Vec3& arbUp);
	static Mat4 perspective(float fov, float aspectRatio, float near, float far);

private:
	Repr _repr;
};

Mat4 operator*(double lambda, const Mat4 &other);

} // namespace maths

#endif // MAT_H
