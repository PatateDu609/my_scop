#include "graphics/shaders.h"

#include <fstream>
#include <iostream>
#include <unordered_map>

namespace graphics::resources {

std::regex Shader::type_detector("#pragma shader_stage[(]([a-zA-Z]+)[)]", std::regex_constants::ECMAScript);

Shader::   Shader(std::string name) : filename(std::move(name)), type(UNKNOWN) {
	   if (!filename.empty() && !load()) {
		   throw std::runtime_error("couldn't load file " + filename);
	   }
}

bool Shader::load() {
	load_file();
	return content.has_value();
}

bool Shader::load(std::string name) {
	filename = std::move(name);
	return load();
}

bool Shader::compile() {
	if (!content) {
		throw std::runtime_error("no content found, please load shader before trying to compile it...");
	}

	auto res = compiler.CompileGlslToSpv(*content, shaderc_glsl_infer_from_source, filename.c_str());
	result	 = std::make_unique<shaderc::CompilationResult<uint32_t>>(std::move(res));
	return result->GetCompilationStatus() == shaderc_compilation_status_success;
}

std::pair<size_t, size_t> Shader::get_num_errors() const {
	if (!result) {
		throw std::runtime_error("no compilation result found, please compile the shader before trying to access it");
	}
	return {result->GetNumErrors(), result->GetNumWarnings()};
}

std::string Shader::errors() const {
	if (!result) {
		throw std::runtime_error("no compilation result found, please compile the shader before trying to access it");
	}

	const auto &[errs, warns] = get_num_errors();
	if (errs == 0 && warns == 0) {
		return "";
	}

	return result->GetErrorMessage();
}

std::vector<uint32_t> Shader::compiled() const {
	if (!result) {
		throw std::runtime_error("no compilation result found, please compile the shader before trying to access it");
	}
	if (result->GetCompilationStatus() != shaderc_compilation_status_success) {
		throw std::runtime_error("compiled didn't succeed, can't access SPIR-V");
	}

	return {result->begin(), result->end()};
}

constexpr Shader::Type Shader::getType() const {
	return type;
}

void Shader::load_file() {
	size_t		size;
	std::string cnt;

	content.reset();
	type = UNKNOWN;

	try {
		std::ifstream ifs(filename);
		ifs.exceptions(std::ios::failbit);
		ifs.exceptions(std::ios::badbit);

		ifs.seekg(0, std::ios::end);
		size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		if (!size)
			return;

		cnt.resize(size);
		ifs.read(cnt.data(), static_cast<std::streamsize>(cnt.size()));

		ifs.close();
	} catch (const std::ios::failure &e) {
		std::cerr << "error(" << e.code() << "): " << filename << ": " << e.what() << std::endl;
		return;
	} catch (const std::exception &e) {
		std::cerr << "error: " << filename << ": " << e.what() << std::endl;
		return;
	}

	content = cnt;

	std::smatch matches;
	if (!std::regex_search(cnt, matches, type_detector) || matches.empty())
		return;

	const static std::unordered_map<std::string, Type> shader_stage_association{
		{"fragment", FRAGMENT},
		{"vertex",   VERTEX	 },
	};

	for (const auto &match : matches) {
		const auto &it = shader_stage_association.find(match.str());

		if (it == shader_stage_association.end())
			continue;

		type = it->second;
		break;
	}
}

} // namespace graphics::resources
