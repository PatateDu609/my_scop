#ifndef SCOP_LINE_OBJECTS_H
#define SCOP_LINE_OBJECTS_H

#include <optional>
#include <string>
#include <vector>

namespace parser {
struct Vertex {
	explicit Vertex(std::vector<std::string> const &args);

	float x{};
	float y{};
	float z{};
};

struct Normal {
	explicit Normal(std::vector<std::string> const &args);

	float i{};
	float j{};
	float k{};
};

struct VertexTexture {
	explicit VertexTexture(std::vector<std::string> const &args);

	float u{};
	float v{};
};

struct Face {
	typedef uint32_t index_type;

	struct Indices {
		explicit Indices(std::string const &arg);

		index_type				  vertex;
		std::optional<index_type> texture;
		std::optional<index_type> normal;

		[[nodiscard]] bool		  validate(bool require_texture, bool require_normal) const;
	};

	explicit Face(std::vector<std::string> const &args);

	std::vector<Indices> vertices;
};
} // namespace parser

#endif // SCOP_LINE_OBJECTS_H
