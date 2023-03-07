#ifndef SCOP_LINE_OBJECTS_H
#define SCOP_LINE_OBJECTS_H

#include <vector>
#include <string>
#include <optional>

namespace parser {
	struct Vertex {
		explicit Vertex(const std::vector<std::string> &args);

		float x{};
		float y{};
		float z{};
		float w{};
	};

	struct VertexTexture {
		explicit VertexTexture(const std::vector<std::string> &args);

		float u{};
		float v{};
		float w{};
	};

	struct Face {
		typedef uint32_t index_type;

		struct Indices {
			explicit Indices(const std::string &arg);

			index_type                vertex;
			std::optional<index_type> texture;
			std::optional<index_type> normal;

			bool validate(bool require_texture, bool require_normal) const;
		};

		explicit Face(const std::vector<std::string> &args);

		std::vector<Indices> vertices;
	};
}

#endif //SCOP_LINE_OBJECTS_H
