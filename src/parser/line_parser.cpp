#include "parser/parser.h"
#include "parser/utils.h"

#include <stdexcept>

using parser::Face;
using parser::Normal;
using parser::Vertex;
using parser::VertexTexture;


Vertex::Vertex(std::vector<std::string> const &args) {
	if (args.size() != 3) {
		throw std::invalid_argument("Vertex expect 3 arguments");
	}

	x = std::stof(args[0]);
	y = std::stof(args[1]);
	z = std::stof(args[2]);
}


Normal::Normal(std::vector<std::string> const &args) {
	if (args.size() != 3) {
		throw std::invalid_argument("Normal expects exactly 3 arguments");
	}

	i = std::stof(args[0]);
	j = std::stof(args[1]);
	k = std::stof(args[2]);
}


VertexTexture::VertexTexture(std::vector<std::string> const &args) {
	if (args.size() != 2) {
		throw std::invalid_argument("Vertex expect 2 arguments");
	}

	u = std::stof(args[0]);
	v = std::stof(args[1]);
}


Face::Indices::Indices(std::string const &arg) {
	std::vector<std::string> values;
	split(values, arg, "/");

	if (values.empty() || values[0].empty())
		throw std::invalid_argument("an index argument should contain at least a vertex reference");
	if (values.size() > 3)
		throw std::invalid_argument(
			"an index argument should have between 1-3 arguments (vertex, vertex texture, vertex normal)");

	vertex = static_cast<uint32_t>(std::stoul(values[0]));
	if (values.size() >= 2 && !values[1].empty())
		texture = static_cast<uint32_t>(std::stoul(values[1]));
	if (values.size() == 3 && !values[2].empty())
		normal = static_cast<uint32_t>(std::stoul(values[2]));
}


bool Face::Indices::validate(bool require_texture, bool require_normal) const {
	return texture.has_value() == require_texture && normal.has_value() == require_normal;
}


Face::Face(std::vector<std::string> const &args) {
	if (args.size() != 3 && args.size()!= 4)
		throw std::invalid_argument("only triangles and quads are currently handled");

	bool first			 = true;
	bool require_texture = false;
	bool require_normal	 = false;
	for (auto const &arg : args) {
		vertices.emplace_back(arg);

		if (first) {
			if (vertices.back().normal.has_value())
				require_normal = true;
			if (vertices.back().texture.has_value())
				require_texture = true;
			first = false;
		}

		if (!vertices.back().validate(require_texture, require_normal))
			throw std::invalid_argument("a face element must be consistent (if an optional element is provided "
										"it should be provided for all vertices)");
	}
}
