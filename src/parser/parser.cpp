#include "parser/parser.h"

#include "parser/utils.h"

#include <fstream>
#include <sstream>

parser::File parser::file;

void		 parser::parse(std::string const &filename) {
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

	file.resolve();
	file.triangulate();
}

void parser::File::resolve() {
	resolved.clear();
	resolved.reserve(faces.size());

	for (auto const &raw : faces) {
		ResolvedFace cur{};

		for (auto const &[vertex_idx, tex_idx, normal_idx] : raw.vertices) {
			ResolvedFace::ResolvedVertex resolvedVertex;
			auto const &[x, y, z] = vertices.at(vertex_idx - 1);
			resolvedVertex.coords = std::make_tuple(x, y, z);

			if (tex_idx) {
				auto const &[tu, tv] = texture_coordinates.at(tex_idx.value() - 1);
				resolvedVertex.tex	 = std::make_tuple(tu, tv);
			}

			if (normal_idx) {
				auto const &[ni, nj, nk] = normals.at(normal_idx.value() - 1);
				resolvedVertex.normal	 = std::make_tuple(ni, nj, nk);
			}

			cur.vertices.push_back(resolvedVertex);
		}

		cur.vertices.shrink_to_fit();
		resolved.push_back(cur);
	}
}

void parser::File::triangulate() {
	using ResolvedVertex = ResolvedFace::ResolvedVertex;
	std::vector<ResolvedFace> triangulated{};

	auto					  triangle = [](ResolvedVertex const &a, ResolvedVertex const &b, ResolvedVertex const &c) {
		 ResolvedFace triangle;

		 triangle.vertices.push_back(a);
		 triangle.vertices.push_back(b);
		 triangle.vertices.push_back(c);

		 return triangle;
	};

	triangulated.reserve(resolved.size() * 2);
	for (auto const &face : resolved) {
		if (face.vertices.size() == 3) {
			triangulated.push_back(face);
			continue;
		} else if (face.vertices.size() != 4) {
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
	resolved.swap(triangulated);
}
