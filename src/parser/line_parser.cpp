#include "parser/parser.h"
#include "parser/utils.h"
#include <stdexcept>

using parser::Vertex;
using parser::VertexTexture;
using parser::Face;


Vertex::Vertex(const std::vector<std::string> &args) {
	if (args.size() != 3 && args.size() != 4) {
		throw std::invalid_argument("Vertex expect 3 or 4 arguments");
	}

	std::vector<float *> values{
			&x,
			&y,
			&z,
	};

	w = 1;
	if (args.size() == 4)
		values.push_back(&w);

	for (size_t i = 0; i < args.size(); i++) {
		*values[i] = std::stof(args[i]);
	}
}


VertexTexture::VertexTexture(const std::vector<std::string> &args) {
	if (args.size() != 2 && args.size() != 3) {
		throw std::invalid_argument("Vertex expect 2 or 3 arguments");
	}

	std::vector<float *> values{
			&u,
			&v,
	};

	w = 0;
	if (args.size() == 3)
		values.push_back(&w);

	for (size_t i = 0; i < args.size(); i++) {
		*values[i] = std::stof(args[i]);
	}
}


Face::Indices::Indices(const std::string &arg) {
	std::vector<std::string> values;
	split(values, arg, "/");

	if (values.empty() || values[0].empty())
		throw std::invalid_argument("an index argument should contain at least a vertex reference");
	if (values.size() > 3)
		throw std::invalid_argument(
				"an index argument should have between 1-3 arguments (vertex, vertex texture, vertex normal)");

	vertex      = static_cast<uint32_t>(std::stoul(values[0]));
	if (values.size() >= 2 && !values[1].empty())
		texture = static_cast<uint32_t>(std::stoul(values[1]));
	if (values.size() == 3 && !values[2].empty())
		normal  = static_cast<uint32_t>(std::stoul(values[2]));
}


bool Face::Indices::validate(bool require_texture, bool require_normal) const {
	return texture.has_value() == require_texture && normal.has_value() == require_normal;
}


Face::Face(const std::vector<std::string> &args) {
	if (args.size() < 3)
		throw std::invalid_argument("a minimum of 3 vertices is required");

	bool            first           = true;
	bool            require_texture = false;
	bool            require_normal  = false;
	for (const auto &arg: args) {
		vertices.emplace_back(arg);

		if (first) {
			if (vertices.back().normal.has_value())
				require_normal = true;
			if (vertices.back().texture.has_value())
				require_texture = true;
			first              = false;
		}

		if (!vertices.back().validate(require_texture, require_normal))
			throw std::invalid_argument("a face element must be consistent (if an optional element is provided "
			                            "it should be provided for all vertices)");
	}
}