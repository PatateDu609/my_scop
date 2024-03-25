#include "maths/mat.h"

#include "maths/vec.h"

namespace maths {

// clang-format off
Mat4 Mat4::identity(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

Mat4::Mat4(const InternalType c11, const InternalType c12, const InternalType c13, const InternalType c14,
		   const InternalType c21, const InternalType c22, const InternalType c23, const InternalType c24,
		   const InternalType c31, const InternalType c32, const InternalType c33, const InternalType c34,
		   const InternalType c41, const InternalType c42, const InternalType c43, const InternalType c44)
	: Mat4(
		{c11, c12, c13, c14},
		{c21, c22, c23, c24},
		{c31, c32, c33, c34},
		{c41, c42, c43, c44})
{}
// clang-format on

Mat4::Mat4(const Line &l1, const Line &l2, const Line &l3, const Line &l4) : _repr{l1, l2, l3, l4} {
}

const Mat4::Line &Mat4::operator[](const size_t idx) const {
	return _repr[idx];
}

Mat4::Line &Mat4::operator[](const size_t idx) {
	return _repr[idx];
}

Mat4 Mat4::operator+(const Mat4 &other) const {
	Mat4 res(*this);
	res += other;
	return res;
}

Mat4 &Mat4::operator+=(const Mat4 &other) {
	_repr[0][0] += other[0][0];
	_repr[0][1] += other[0][1];
	_repr[0][2] += other[0][2];
	_repr[0][3] += other[0][3];

	_repr[1][0] += other[1][0];
	_repr[1][1] += other[1][1];
	_repr[1][2] += other[1][2];
	_repr[1][3] += other[1][3];

	_repr[2][0] += other[2][0];
	_repr[2][1] += other[2][1];
	_repr[2][2] += other[2][2];
	_repr[2][3] += other[2][3];

	_repr[3][0] += other[3][0];
	_repr[3][1] += other[3][1];
	_repr[3][2] += other[3][2];
	_repr[3][3] += other[3][3];

	return *this;
}

Mat4 Mat4::operator-(const Mat4 &other) const {
	Mat4 res(*this);
	res -= other;
	return res;
}

Mat4 &Mat4::operator-=(const Mat4 &other) {
	*this += -other;
	return *this;
}

Mat4 Mat4::operator*(const Mat4 &other) const {
	Mat4 res(*this);
	res *= other;
	return res;
}

Mat4 &Mat4::operator*=(const Mat4 &other) {
	const InternalType a11 = _repr[0][0], a12 = _repr[0][1], a13 = _repr[0][2], a14 = _repr[0][3];
	const InternalType a21 = _repr[1][0], a22 = _repr[1][1], a23 = _repr[1][2], a24 = _repr[1][3];
	const InternalType a31 = _repr[2][0], a32 = _repr[2][1], a33 = _repr[2][2], a34 = _repr[2][3];
	const InternalType a41 = _repr[3][0], a42 = _repr[3][1], a43 = _repr[3][2], a44 = _repr[3][3];

	const InternalType b11 = other[0][0], b12 = other[0][1], b13 = other[0][2], b14 = other[0][3];
	const InternalType b21 = other[1][0], b22 = other[1][1], b23 = other[1][2], b24 = other[1][3];
	const InternalType b31 = other[2][0], b32 = other[2][1], b33 = other[2][2], b34 = other[2][3];
	const InternalType b41 = other[3][0], b42 = other[3][1], b43 = other[3][2], b44 = other[3][3];

	_repr[0][0] = a11 * b11 + a12 * b21 + a13 * b31 + a14 * b41;
	_repr[0][1] = a11 * b12 + a12 * b22 + a13 * b32 + a14 * b42;
	_repr[0][2] = a11 * b13 + a12 * b23 + a13 * b33 + a14 * b43;
	_repr[0][0] = a11 * b14 + a12 * b24 + a13 * b34 + a14 * b44;

	_repr[1][0] = a21 * b11 + a22 * b21 + a23 * b31 + a24 * b41;
	_repr[1][1] = a21 * b12 + a22 * b22 + a23 * b32 + a24 * b42;
	_repr[1][2] = a21 * b13 + a22 * b23 + a23 * b33 + a24 * b43;
	_repr[1][0] = a21 * b14 + a22 * b24 + a23 * b34 + a24 * b44;

	_repr[2][0] = a31 * b11 + a32 * b21 + a33 * b31 + a34 * b41;
	_repr[2][1] = a31 * b12 + a32 * b22 + a33 * b32 + a34 * b42;
	_repr[2][2] = a31 * b13 + a32 * b23 + a33 * b33 + a34 * b43;
	_repr[2][0] = a31 * b14 + a32 * b24 + a33 * b34 + a34 * b44;

	_repr[3][0] = a41 * b11 + a42 * b21 + a43 * b31 + a44 * b41;
	_repr[3][1] = a41 * b12 + a42 * b22 + a43 * b32 + a44 * b42;
	_repr[3][2] = a41 * b13 + a42 * b23 + a43 * b33 + a44 * b43;
	_repr[3][0] = a41 * b14 + a42 * b24 + a43 * b34 + a44 * b44;

	return *this;
}

Mat4 Mat4::operator*(const InternalType lambda) const {
	Mat4 res(*this);
	res *= lambda;
	return res;
}

Mat4 &Mat4::operator*=(const InternalType lambda) {
	_repr[0][0] *= lambda;
	_repr[0][1] *= lambda;
	_repr[0][2] *= lambda;
	_repr[0][3] *= lambda;

	_repr[1][0] *= lambda;
	_repr[1][1] *= lambda;
	_repr[1][2] *= lambda;
	_repr[1][3] *= lambda;

	_repr[2][0] *= lambda;
	_repr[2][1] *= lambda;
	_repr[2][2] *= lambda;
	_repr[2][3] *= lambda;

	_repr[3][0] *= lambda;
	_repr[3][1] *= lambda;
	_repr[3][2] *= lambda;
	_repr[3][3] *= lambda;

	return *this;
}

Mat4 Mat4::operator/(const InternalType lambda) const {
	Mat4 res(*this);
	res /= lambda;
	return res;
}

Mat4 &Mat4::operator/=(const InternalType lambda) {
	if (lambda == 0) {
		throw std::invalid_argument("invalid division by 0");
	}
	*this *= 1 / lambda;
	return *this;
}

Mat4 Mat4::operator-() const {
	return -1 * *this;
}

Mat4 operator*(const double lambda, const Mat4 &other) {
	return other * lambda;
}

Mat4 Mat4::rotate(const InternalType angle, const Vec3 &u) {
	float ux, uy, uz;
	auto  uNormalized	 = u.normalized();
	std::tie(ux, uy, uz) = uNormalized;

	const float C		 = std::cos(angle);
	const float S		 = std::sin(angle);
	const float T		 = 1 - C;

	Mat4		m		 = identity;

	m[0][0]				 = C + ux * ux * T;
	m[0][1]				 = ux * uy * T - uz * S;
	m[0][2]				 = ux * uz * T + uy * S;

	m[1][0]				 = ux * uy * T + uz * S;
	m[1][1]				 = C + uy * uy * T;
	m[1][2]				 = uy * uz * T - ux * S;

	m[2][0]				 = ux * uz * T - uy * S;
	m[2][1]				 = uy * uz * T + ux * S;
	m[2][2]				 = C + uz * uz * T;

	return m;
}

Mat4 Mat4::lookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &arbUp) {
	Vec3 forward = (center - eye).normalized();
	Vec3 right	 = forward.cross(arbUp).normalized();
	Vec3 up		 = right.cross(forward);

	Mat4 lookAt	 = identity;

	lookAt[0][0] = right.x();
	lookAt[1][0] = right.y();
	lookAt[2][0] = right.z();

	lookAt[0][1] = up.x();
	lookAt[1][1] = up.y();
	lookAt[2][1] = up.z();

	lookAt[0][2] = -forward.x();
	lookAt[1][2] = -forward.y();
	lookAt[2][2] = -forward.z();

	lookAt[3][0] = -(right * eye);
	lookAt[3][1] = -(up * eye);
	lookAt[3][2] = (forward * eye);

	return lookAt;
}

Mat4 Mat4::perspective(const float fov, const float aspectRatio, float near, float far) {
	const float tan_half_fov = std::tan(fov * 0.5f);

	Mat4		projMatrix;

	projMatrix[0][0] = 1.0f / (aspectRatio * tan_half_fov);
	projMatrix[1][1] = 1.0f / tan_half_fov;
	projMatrix[2][2] = -(far + near) / (far - near);
	projMatrix[2][3] = -1.0f;
	projMatrix[3][2] = -(2.0f * far * near) / (far - near);

#ifdef MATH_FORCE_DEPTH_ZERO_TO_ONE
	// Adjust projection matrix for [0, 1] depth range
	projMatrix[2][2] = -2.0f / (far - near);
	projMatrix[2][3] = -(far + near) / (far - near);
#endif

	return projMatrix;
}


} // namespace maths
