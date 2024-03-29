#include "parser/utils.h"

#include <ranges>

void parser::split(std::vector<std::string> &args, const std::string_view &line, const std::string_view &delimiter) {
	auto splitted = std::views::split(line, delimiter);
	for (const auto &word : splitted) {
		args.emplace_back(std::string_view{word});
	}
}
