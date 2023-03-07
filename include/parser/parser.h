#ifndef SCOP_PARSER_H
#define SCOP_PARSER_H

#include <string>
#include <vector>
#include "line_objects.h"

namespace parser {
	void parse(const std::string &filename);


	class ifs_error : public std::exception {
	public:
		explicit ifs_error(const std::string &filename);

		[[nodiscard]] const char *what() const override;

	private:
		std::string err_str;
	};

	class File {
		friend void parse(const std::string &filename);

		std::vector<Vertex>        vertices;
		std::vector<VertexTexture> texture_coordinates;
		std::vector<Face>          faces;
	};

	extern File file;
}

#endif //SCOP_PARSER_H
