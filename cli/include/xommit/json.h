#pragma once

#include <initializer_list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace xommit {

class Json {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Type type = Type::Null;
    bool boolean = false;
    double number = 0.0;
    std::string string;
    std::vector<Json> array;
    std::map<std::string, Json> object;

    Json() = default;
    Json(bool value);
    Json(int value);
    Json(double value);
    Json(const char* value);
    Json(std::string value);

    static Json parse(const std::string& text);
    std::string dump() const;

    bool is_null() const;
    bool is_object() const;
    bool is_array() const;
    bool is_string() const;

    const Json* find(const std::string& key) const;
    std::string get_string(const std::string& key, const std::string& fallback = "") const;
    double get_number(const std::string& key, double fallback = 0.0) const;
    bool get_bool(const std::string& key, bool fallback = false) const;

    const Json& at(size_t index) const;
    size_t size() const;
};

Json json_array(std::initializer_list<Json> values);
Json json_object(std::initializer_list<std::pair<const std::string, Json>> values);

}
