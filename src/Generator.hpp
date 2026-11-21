#pragma once
#include <string>
#include <string_view>

namespace dlgen {
enum class Difficulty { Easy, Normal, Hard, Insane, Demon };
enum class Length { Medium, Long, XL };
struct Settings {
    Difficulty difficulty = Difficulty::Normal;
    Length length = Length::Medium;
    bool orbs = true;
    bool triggers = true;
};
struct ParseResult {
    Settings settings;
    bool valid = true;
    std::string error;
};
ParseResult parseKeywords(std::string_view keywords);
std::string_view difficultyName(Difficulty difficulty);
std::string_view lengthName(Length length);
int targetDurationSeconds(Length length);
} // namespace dlgen
