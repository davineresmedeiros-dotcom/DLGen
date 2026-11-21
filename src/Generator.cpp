#include "Generator.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace dlgen {
namespace {
std::string normalize(std::string_view input) {
    std::string result(input);
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}
bool parseBool(std::string const& value, bool& output) {
    if (value == "ativo" || value == "active" || value == "sim" ||
        value == "yes" || value == "true" || value == "on") {
        output = true; return true;
    }
    if (value == "nao-ativo" || value == "não-ativo" || value == "inactive" ||
        value == "nao" || value == "não" || value == "no" ||
        value == "false" || value == "off") {
        output = false; return true;
    }
    return false;
}
} // namespace

ParseResult parseKeywords(std::string_view keywords) {
    ParseResult result;
    std::istringstream stream{std::string(keywords)};
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) tokens.push_back(normalize(token));

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        token = tokens[i];
        if (token == "easy") result.settings.difficulty = Difficulty::Easy;
        else if (token == "normal") result.settings.difficulty = Difficulty::Normal;
        else if (token == "hard") result.settings.difficulty = Difficulty::Hard;
        else if (token == "insane") result.settings.difficulty = Difficulty::Insane;
        else if (token == "demon") result.settings.difficulty = Difficulty::Demon;
        else if (token == "medium" || token == "médium") result.settings.length = Length::Medium;
        else if (token == "long") result.settings.length = Length::Long;
        else if (token == "xl") result.settings.length = Length::XL;
        else if (token == "orbs" || token == "triggers") {
            bool isOrb = token == "orbs";
            if (++i >= tokens.size()) {
                result.valid = false;
                result.error = "Falta o valor para " + token;
                return result;
            }
            bool value = false;
            if (!parseBool(tokens[i], value)) {
                result.valid = false;
                result.error = "Valor inválido para " + token + ": " + tokens[i];
                return result;
            }
            if (isOrb) result.settings.orbs = value;
            else result.settings.triggers = value;
        }
        else if (token.starts_with("orbs=") || token.starts_with("triggers=")) {
            bool isOrb = token.starts_with("orbs=");
            bool value = false;
            auto equal = token.find('=');
            if (!parseBool(token.substr(equal + 1), value)) {
                result.valid = false;
                result.error = "Valor inválido: " + token;
                return result;
            }
            if (isOrb) result.settings.orbs = value;
            else result.settings.triggers = value;
        }
        else {
            result.valid = false;
            result.error = "Palavra-chave desconhecida: " + token;
            return result;
        }
    }
    return result;
}

std::string_view difficultyName(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy: return "easy";
        case Difficulty::Normal: return "normal";
        case Difficulty::Hard: return "hard";
        case Difficulty::Insane: return "insane";
        case Difficulty::Demon: return "demon";
    }
    return "normal";
}
std::string_view lengthName(Length length) {
    switch (length) {
        case Length::Medium: return "medium";
        case Length::Long: return "long";
        case Length::XL: return "XL";
    }
    return "medium";
}
int targetDurationSeconds(Length length) {
    switch (length) {
        case Length::Medium: return 60;
        case Length::Long: return 120;
        case Length::XL: return 300;
    }
    return 60;
}
} // namespace dlgen
