#ifndef SCOP_SHADERS_H
#define SCOP_SHADERS_H

#include <memory>
#include <optional>
#include <regex>
#include <shaderc/shaderc.hpp>
#include <string>
#include <vector>

namespace graphics::resources {

class Shader final {
public:
	enum Type {
		UNKNOWN,
		VERTEX,
		FRAGMENT,
	};

								 Shader(const Shader &)	   = delete;
	Shader						&operator=(const Shader &) = delete;

	explicit					 Shader(std::string name = "");

	auto						 load() -> bool;
	auto						 load(std::string name) -> bool;
	auto						 compile() -> bool;

	[[nodiscard]] auto			 get_num_errors() const -> std::pair<size_t, size_t>;
	[[nodiscard]] auto			 errors() const -> std::string;
	[[nodiscard]] auto			 compiled() const -> std::vector<uint32_t>;

	[[nodiscard]] constexpr auto getType() const -> Type;

private:
	void												  load_file();

	std::string											  filename{};
	Type												  type{};

	std::optional<std::string>							  content{};

	shaderc::Compiler									  compiler;
	std::unique_ptr<shaderc::CompilationResult<uint32_t>> result;

	static std::regex									  type_detector;
};

} // namespace graphics::resources

#endif // SCOP_SHADERS_H
