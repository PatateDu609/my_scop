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
	friend void parse(const std::string &filename);

public:
	struct ResolvedFace {
		struct ResolvedVertex {
			std::tuple<float, float, float>				   coords;
			std::optional<std::tuple<float, float>>		   tex;
			std::optional<std::tuple<float, float, float>> normal;
		};

		std::vector<ResolvedVertex> vertices{};
	};

private:
	void					   resolve();
	void					   triangulate();

	std::vector<Vertex>		   vertices;
	std::vector<VertexTexture> texture_coordinates;
	std::vector<Normal>		   normals;
	std::vector<Face>		   faces;

	std::vector<ResolvedFace>  resolved;
};

extern File file;
} // namespace parser

#endif // SCOP_PARSER_H
