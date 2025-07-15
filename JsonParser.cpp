#include <stdexcept>
#include <algorithm>
#include "JsonParser.h"

// Helper function to trim whitespace
std::string JsonParser::trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Helper function to strip quotes
std::string JsonParser::strip_quotes(const std::string &str) {
    if (str.size() > 1 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

// Parse JSON string into key-value pairs
void JsonParser::parse(const std::string &json) {
    data.clear();

    if (json.empty()) {
        throw std::invalid_argument("Empty JSON string");
    }

    std::string trimmed = trim(json);

    if (trimmed.front() != '{' || trimmed.back() != '}') {
        throw std::invalid_argument("Invalid JSON format: Must start with '{' and end with '}'");
    }

    trimmed = trimmed.substr(1, trimmed.size() - 2);

    size_t pos = 0;
    while (pos < trimmed.size()) {
        size_t key_start = trimmed.find('"', pos);
        if (key_start == std::string::npos) break;
        size_t key_end = trimmed.find('"', key_start + 1);
        if (key_end == std::string::npos) break;
        std::string key = trimmed.substr(key_start + 1, key_end - key_start - 1);

        size_t colon = trimmed.find(':', key_end + 1);
        if (colon == std::string::npos) {
            throw std::invalid_argument("Invalid JSON format: Missing ':' after key");
        }

        size_t value_start = colon + 1;
        size_t value_end = trimmed.find(',', value_start);
        if (value_end == std::string::npos) value_end = trimmed.size();
        std::string value = trim(trimmed.substr(value_start, value_end - value_start));

        value = strip_quotes(value);

        data[key] = value;

        pos = value_end + 1;
    }
}

// Access value by key
std::string JsonParser::operator[](const std::string &key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        throw std::out_of_range("Key not found: " + key);
    }
    return it->second;
}

// Check if a key exists
bool JsonParser::contains(const std::string &key) const {
    return data.find(key) != data.end();
}
