#ifndef VEC_H
#define VEC_H

#include <array>

namespace maths {

class Vec2 {
public:
	explicit			Vec2(float x = 0, float y = 0);

	Vec2				operator+(const Vec2 &other) const;
	Vec2			   &operator+=(const Vec2 &other);

	Vec2				operator-(const Vec2 &other) const;
	Vec2			   &operator-=(const Vec2 &other);

	Vec2				operator*(float lambda) const;
	Vec2			   &operator*=(float lambda);
	float				operator*(const Vec2 &other) const;

	float			   &x();
	float				x() const;

	float			   &y();
	float				y() const;

	[[nodiscard]] float norm2() const;
	[[nodiscard]] float norm() const;

	// ReSharper disable once CppNonExplicitConversionOperator
	template <typename T, typename U>
	operator std::tuple<T, U>() {
		return std::make_tuple(x(), y());
	}

private:
	typedef std::array<float, 3>	Repr;
	typedef std::pair<float, float> NormPair;

	float							compute_norm(bool use_sqrt) const;

	Repr							_repr;
};
Vec2 operator*(float x, const Vec2 &other);


class Vec3 {
public:
	explicit			Vec3(float x = 0, float y = 0, float z = 0);

	Vec3				operator+(const Vec3 &other) const;
	Vec3			   &operator+=(const Vec3 &other);

	Vec3				operator-(const Vec3 &other) const;
	Vec3			   &operator-=(const Vec3 &other);

	Vec3			   &operator-();
	Vec3				operator-() const;

	Vec3				operator*(float lambda) const;
	Vec3			   &operator*=(float lambda);
	float				operator*(const Vec3 &other) const;
	Vec3				cross(const Vec3 &other) const;
	Vec3			   &crossed(const Vec3 &other);

	float			   &x();
	float				x() const;

	float			   &y();
	float				y() const;

	float			   &z();
	float				z() const;

	[[nodiscard]] float norm2() const;
	[[nodiscard]] float norm() const;

	Vec3			   &normalize();
	Vec3				normalized() const;


	// ReSharper disable once CppNonExplicitConversionOperator
	template <typename T, typename U, typename V>
	operator std::tuple<T, U, V>() {
		return {x(), y(), z()};
	}

private:
	typedef std::array<float, 3>	Repr;
	typedef std::pair<float, float> NormPair;

	float							compute_norm(bool use_sqrt) const;

	Repr							_repr;
};

Vec3 operator*(float x, const Vec3 &other);

} // namespace maths

#endif // VEC_H
