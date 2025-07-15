#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <unordered_map>

class JsonParser {
private:
    std::unordered_map<std::string, std::string> data;

    // Helper function to trim whitespace
    static std::string trim(const std::string &str);

    // Helper function to strip quotes
    static std::string strip_quotes(const std::string &str);

public:
    // Parse JSON string into key-value pairs
    void parse(const std::string &json);

    // Access value by key
    std::string operator[](const std::string &key) const;

    // Check if a key exists
    bool contains(const std::string &key) const;
};

#endif
