#include "parser/parser.h"

#include "parser/utils.h"

#include <fstream>
#include <iostream>
#include <sstream>

parser::File parser::file;

void		 parser::parse(const std::string &filename) {
	std::ifstream ifs(filename);

	if (!ifs)
		throw parser::ifs_error(filename);

	file.vertices.clear();

	std::string				 line;
	std::vector<std::string> args;
	while (std::getline(ifs, line)) {
		split(args, line);
		std::string id = args[0];
		args.erase(args.begin());

		if (id == "v")
			file.vertices.emplace_back(args);
		else if (id == "vt")
			file.texture_coordinates.emplace_back(args);
		else if (id == "vn")
			file.normals.emplace_back(args);
		else if (id == "f")
			file.faces.emplace_back(args);
	}
	ifs.close();

	file.faces.shrink_to_fit();
	file.vertices.shrink_to_fit();
	file.texture_coordinates.shrink_to_fit();

	file.triangulate();
}

void parser::File::triangulate() {
	std::vector<Face> triangulated{};

	auto			  printFace = [](const std::string &title, const std::vector<Face> &faces) {
		 std::cout << title << ":\n";

		 for (const auto &face : faces) {
			 std::cout << "\t-";

			 for (const auto &v : face.vertices) {
				 std::cout << " " << v.vertex + 1;
			 }
			 std::cout << "\n";
		 }

		 std::cout << std::flush;
	};

	auto triangle = [](const Face::Indices &a, const Face::Indices &b, const Face::Indices &c) {
		Face triangleFace{};

		triangleFace.vertices.push_back(b);
		triangleFace.vertices.push_back(c);
		triangleFace.vertices.push_back(a);

		return triangleFace;
	};

	printFace("Faces before triangulation:", faces);

	triangulated.reserve(faces.size() * 2);
	for (const auto &face : faces) {
		if (face.vertices.size() == 3) {
			triangulated.push_back(face);
			continue;
		}
		if (face.vertices.size() != 4) {
			std::ostringstream oss;

			if (face.vertices.size() == 1)
				oss << "got face with 1 vertex...";
			else if (face.vertices.size() == 2)
				oss << "got face with 2 sides, it would be a segment...";
			else
				oss << "got face with " << face.vertices.size() << " sides, it shouldn't happen as it's not handled";

			throw std::runtime_error(oss.str());
		}

		// 0-----------1
		// |         / |
		// |       /   |
		// |     /     |
		// |   /       |
		// | /         |
		// 3-----------2

		auto v = face.vertices;

		triangulated.emplace_back(triangle(v[0], v[1], v[3]));
		triangulated.emplace_back(triangle(v[3], v[1], v[2]));
	}

	triangulated.shrink_to_fit();
	this->triangulated.swap(triangulated);

	if constexpr (DEBUG) {
		printFace("Faces after triangulation", this->triangulated);
	}
}
