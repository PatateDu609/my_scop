#include "maths/vec.h"

namespace maths {

/**************************************************************************************/
//                                        Vec2                                        //
/**************************************************************************************/

Vec2::Vec2(float x, float y) : _repr{x, y} {
}

Vec2 Vec2::operator+(const Vec2 &other) const {
	return Vec2{x() + other.x(), y() + other.y()};
}

Vec2 &Vec2::operator+=(const Vec2 &other) {
	x() += other.x();
	y() += other.y();
	return *this;
}

Vec2 Vec2::operator-(const Vec2 &other) const {
	return Vec2{x() - other.x(), y() - other.y()};
}

Vec2 &Vec2::operator-=(const Vec2 &other) {
	x() -= other.x();
	y() -= other.y();
	return *this;
}

Vec2 Vec2::operator*(const float lambda) const {
	return Vec2{x() * lambda, y() * lambda};
}

Vec2 &Vec2::operator*=(const float lambda) {
	x() *= lambda;
	y() *= lambda;
	return *this;
}

float Vec2::operator*(const Vec2 &other) const {
	return x() * other.x() + y() * other.y();
}

float &Vec2::x() {
	return std::get<0>(_repr);
}

float Vec2::x() const {
	return std::get<0>(_repr);
}

float &Vec2::y() {
	return std::get<1>(_repr);
}

float Vec2::y() const {
	return std::get<1>(_repr);
}

Vec2 operator*(const float x, const Vec2 &other) {
	return other * x;
}

float Vec2::norm2() const {
	return compute_norm(false);
}

float Vec2::norm() const {
	return compute_norm(true);
}

float Vec2::compute_norm(const bool use_sqrt) const {
	// .first => Repr used to compute norm
	// .second => Cached norm in which .first will be the Scalar Product (norm * norm) and .second will be the squared root of .first
	static std::pair<Repr, NormPair> cachedNorm{
		{0, 0},
		{0, 0}
	  };

	if (cachedNorm.first != _repr) {
		const float normSquared	 = *this * *this;
		cachedNorm.first		 = _repr;
		cachedNorm.second.first	 = normSquared;
		cachedNorm.second.second = std::sqrt(normSquared);
	}

	return use_sqrt ? cachedNorm.second.second : cachedNorm.second.first;
}

/**************************************************************************************/
//                                        Vec3                                        //
/**************************************************************************************/

Vec3::Vec3(const float x, const float y, const float z) : _repr{x, y, z} {
}

Vec3 Vec3::operator+(const Vec3 &other) const {
	return Vec3{x() + other.x(), y() + other.y(), z() + other.z()};
}

Vec3 &Vec3::operator+=(const Vec3 &other) {
	x() += other.x();
	y() += other.y();
	z() += other.z();
	return *this;
}

Vec3 Vec3::operator-(const Vec3 &other) const {
	return Vec3{x() - other.x(), y() - other.y(), z() - other.z()};
}

Vec3 &Vec3::operator-=(const Vec3 &other) {
	x() -= other.x();
	y() -= other.y();
	z() -= other.z();
	return *this;
}

Vec3 &Vec3::operator-() {
	x() = -x();
	y() = -y();
	z() = -z();
	return *this;
}
Vec3 Vec3::operator-() const {
	return Vec3(-x(), -y(), -z());
}

Vec3 Vec3::operator*(const float lambda) const {
	return Vec3{x() * lambda, y() * lambda, z() * lambda};
}

Vec3 &Vec3::operator*=(const float lambda) {
	x() *= lambda;
	y() *= lambda;
	z() *= lambda;
	return *this;
}

float Vec3::operator*(const Vec3 &other) const {
	return x() * other.x() + y() * other.y() + z() * other.z();
}

Vec3 Vec3::cross(const Vec3 &other) const {
	Vec3 res(*this);
	return res.crossed(other);
}

Vec3 &Vec3::crossed(const Vec3 &other) {
	// yz' − zy', zx' − xz', xy' − yx'
	const double cur_x = x(), cur_y = y(), cur_z = z();

	x() = cur_y * other.z() - cur_z * other.y();
	y() = cur_z * other.x() - cur_x * other.z();
	z() = cur_x * other.y() - cur_y * other.x();
	return *this;
}

float &Vec3::x() {
	return std::get<0>(_repr);
}

float Vec3::x() const {
	return std::get<0>(_repr);
}

float &Vec3::y() {
	return std::get<1>(_repr);
}

float Vec3::y() const {
	return std::get<1>(_repr);
}

float &Vec3::z() {
	return std::get<2>(_repr);
}

float Vec3::z() const {
	return std::get<2>(_repr);
}

Vec3 operator*(const float x, const Vec3 &other) {
	return other * x;
}

float Vec3::norm2() const {
	return compute_norm(false);
}

float Vec3::norm() const {
	return compute_norm(true);
}

float Vec3::compute_norm(const bool use_sqrt) const {
	// .first => Repr used to compute norm
	// .second => Cached norm in which .first will be the Scalar Product (norm * norm) and .second will be the squared root of .first
	static std::pair<Repr, NormPair> cachedNorm{
		{0, 0, 0},
		   {0, 0}
	 };

	if (cachedNorm.first != _repr) {
		const float normSquared	 = *this * *this;
		cachedNorm.first		 = _repr;
		cachedNorm.second.first	 = normSquared;
		cachedNorm.second.second = std::sqrt(normSquared);
	}

	return use_sqrt ? cachedNorm.second.second : cachedNorm.second.first;
}

Vec3 &Vec3::normalize() {
	const float n  = norm();
	x()			  /= n;
	y()			  /= n;
	z()			  /= n;
	return *this;
}

Vec3 Vec3::normalized() const {
	return Vec3(*this).normalize();
}

} // namespace maths
