#ifndef UTILS_H
#define UTILS_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace maths {

constexpr float deg(const float angle) {
	constexpr float ratio = 180.f / M_PI;
	return angle * ratio;
}

constexpr float rad(const float angle) {
	constexpr float ratio = M_PI / 180.f;
	return angle * ratio;
}

template <typename T>
concept Matrix = requires(T a, size_t idx) { a[idx]; };

template <Matrix Mat>
void display_mat(const std::string &title, const Mat &mat, const size_t m, const size_t n) {
	std::ostringstream oss;

	oss << std::setprecision(3);
	for (size_t i = 0; i < m; i++) {
		for (size_t j = 0; j < n; j++) {
			oss << std::setw(7) << mat[i][j];
		}
		oss << "\n";
	}

	std::cout << "---------------------------------------------------------------------------------------------------------------------\n";
	std::cout << "Matrix " << title << ":\n" << oss.str();
	std::cout << "---------------------------------------------------------------------------------------------------------------------\n";
	std::cout << std::flush;
};

} // namespace maths

#endif // UTILS_H
