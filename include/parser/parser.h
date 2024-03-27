#ifndef SCOP_PARSER_H
#define SCOP_PARSER_H

#include "line_objects.h"

#include <string>
#include <vector>

namespace parser {
void parse(const std::string &filename);

class ifs_error : public std::exception {
public:
	explicit				  ifs_error(const std::string &filename);

	[[nodiscard]] const char *what() const throw() override;

private:
	std::string err_str;
};

class File {
	friend void				   parse(const std::string &filename);

	void					   triangulate();

	std::vector<Vertex>		   vertices;
	std::vector<VertexTexture> texture_coordinates;
	std::vector<Normal>		   normals;
	std::vector<Face>		   faces;
	std::vector<Face>		   triangulated;

public:
	decltype(triangulated)::const_iterator begin() const {
		return triangulated.begin();
	}

	decltype(triangulated)::const_iterator end() const {
		return triangulated.end();
	}

	decltype(vertices)::const_reference vertex(const size_t i) const {
		return vertices[i];
	}

	decltype(normals)::const_reference normal(const size_t i) const {
		return normals[i];
	}

	decltype(texture_coordinates)::const_reference texCoord(const size_t i) const {
		return texture_coordinates[i];
	}
};

extern File file;
} // namespace parser

#endif // SCOP_PARSER_H
