// minimal json parser. string/number/bool/null/array/object
// used for provider requests/responses, config file

#include "xommit/json.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace xommit {

namespace {

class JsonParser {
public:
    explicit JsonParser(const std::string& text) : text_(text) {}

    Json parse() {
        skip_whitespace();
        Json value = parse_value();
        skip_whitespace();
        if (position_ != text_.size()) {
            error("unexpected trailing data");
        }
        return value;
    }

private:
    const std::string& text_;
    size_t position_ = 0;

    char peek() const {
        return position_ < text_.size() ? text_[position_] : '\0';
    }

    char take() {
        if (position_ >= text_.size()) {
            error("unexpected end of input");
        }
        return text_[position_++];
    }

    void expect(char value) {
        if (take() != value) {
            error(std::string("expected '") + value + "'");
        }
    }

    void skip_whitespace() {
        while (position_ < text_.size()) {
            char c = text_[position_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                position_++;
            } else {
                break;
            }
        }
    }

    [[noreturn]] void error(const std::string& message) const {
        throw std::runtime_error("JSON parse error at byte " + std::to_string(position_) + ": " + message);
    }

    Json parse_value() {
        skip_whitespace();
        char c = peek();
        if (c == '{') {
            return parse_object();
        }
        if (c == '[') {
            return parse_array();
        }
        if (c == '"') {
            return Json(parse_string());
        }
        if (c == 't' || c == 'f') {
            return parse_bool();
        }
        if (c == 'n') {
            parse_null();
            return Json();
        }
        if (c == '-' || (c >= '0' && c <= '9')) {
            return Json(parse_number());
        }
        error("unexpected value");
    }

    Json parse_object() {
        expect('{');
        Json result;
        result.type = Json::Type::Object;
        skip_whitespace();
        if (peek() == '}') {
            position_++;
            return result;
        }
        while (true) {
            skip_whitespace();
            if (peek() != '"') {
                error("expected object key");
            }
            std::string key = parse_string();
            skip_whitespace();
            expect(':');
            result.object[key] = parse_value();
            skip_whitespace();
            char c = take();
            if (c == '}') {
                break;
            }
            if (c != ',') {
                error("expected ',' or '}'");
            }
        }
        return result;
    }

    Json parse_array() {
        expect('[');
        Json result;
        result.type = Json::Type::Array;
        skip_whitespace();
        if (peek() == ']') {
            position_++;
            return result;
        }
        while (true) {
            result.array.push_back(parse_value());
            skip_whitespace();
            char c = take();
            if (c == ']') {
                break;
            }
            if (c != ',') {
                error("expected ',' or ']'");
            }
        }
        return result;
    }

    bool parse_bool() {
        if (text_.compare(position_, 4, "true") == 0) {
            position_ += 4;
            return true;
        }
        if (text_.compare(position_, 5, "false") == 0) {
            position_ += 5;
            return false;
        }
        error("invalid literal");
    }

    void parse_null() {
        if (text_.compare(position_, 4, "null") != 0) {
            error("invalid literal");
        }
        position_ += 4;
    }

    double parse_number() {
        size_t start = position_;
        if (peek() == '-') {
            position_++;
        }
        while (position_ < text_.size()) {
            char c = text_[position_];
            if ((c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') {
                position_++;
            } else {
                break;
            }
        }
        std::string token = text_.substr(start, position_ - start);
        char* end = nullptr;
        double value = std::strtod(token.c_str(), &end);
        if (end == token.c_str()) {
            error("invalid number");
        }
        return value;
    }

    static void append_utf8(std::string& out, unsigned int code) {
        if (code <= 0x7F) {
            out += static_cast<char>(code);
        } else if (code <= 0x7FF) {
            out += static_cast<char>(0xC0 | (code >> 6));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else if (code <= 0xFFFF) {
            out += static_cast<char>(0xE0 | (code >> 12));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (code >> 18));
            out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        }
    }

    unsigned int parse_hex4() {
        unsigned int value = 0;
        for (int i = 0; i < 4; i++) {
            char c = take();
            value <<= 4;
            if (c >= '0' && c <= '9') {
                value += static_cast<unsigned int>(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                value += static_cast<unsigned int>(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                value += static_cast<unsigned int>(c - 'A' + 10);
            } else {
                error("invalid unicode escape");
            }
        }
        return value;
    }

    std::string parse_string() {
        expect('"');
        std::string result;
        while (true) {
            char c = take();
            if (c == '"') {
                break;
            }
            if (c != '\\') {
                result += c;
                continue;
            }
            char escape = take();
            switch (escape) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': {
                    unsigned int code = parse_hex4();
                    if (code >= 0xD800 && code <= 0xDBFF && position_ + 1 < text_.size() &&
                        text_[position_] == '\\' && text_[position_ + 1] == 'u') {
                        position_ += 2;
                        unsigned int low = parse_hex4();
                        if (low >= 0xDC00 && low <= 0xDFFF) {
                            code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                        }
                    }
                    append_utf8(result, code);
                    break;
                }
                default:
                    error("invalid escape");
            }
        }
        return result;
    }
};

void escape_string(std::ostringstream& out, const std::string& value) {
    out << '"';
    for (char c : value) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
                } else {
                    out << c;
                }
        }
    }
    out << '"';
}

void dump_value(std::ostringstream& out, const Json& value) {
    switch (value.type) {
        case Json::Type::Null:
            out << "null";
            break;
        case Json::Type::Bool:
            out << (value.boolean ? "true" : "false");
            break;
        case Json::Type::Number:
            if (std::isfinite(value.number) && std::floor(value.number) == value.number &&
                std::abs(value.number) < 1e15) {
                out << static_cast<long long>(value.number);
            } else {
                out << std::setprecision(10) << value.number;
            }
            break;
        case Json::Type::String:
            escape_string(out, value.string);
            break;
        case Json::Type::Array:
            out << '[';
            for (size_t i = 0; i < value.array.size(); i++) {
                if (i > 0) {
                    out << ',';
                }
                dump_value(out, value.array[i]);
            }
            out << ']';
            break;
        case Json::Type::Object:
            out << '{';
            {
                bool first = true;
                for (const auto& entry : value.object) {
                    if (!first) {
                        out << ',';
                    }
                    first = false;
                    escape_string(out, entry.first);
                    out << ':';
                    dump_value(out, entry.second);
                }
            }
            out << '}';
            break;
    }
}

}

Json::Json(bool value) : type(Type::Bool), boolean(value) {}
Json::Json(int value) : type(Type::Number), number(value) {}
Json::Json(double value) : type(Type::Number), number(value) {}
Json::Json(const char* value) : type(Type::String), string(value == nullptr ? "" : value) {}
Json::Json(std::string value) : type(Type::String), string(std::move(value)) {}

Json Json::parse(const std::string& text) {
    return JsonParser(text).parse();
}

std::string Json::dump() const {
    std::ostringstream out;
    dump_value(out, *this);
    return out.str();
}

bool Json::is_null() const { return type == Type::Null; }
bool Json::is_object() const { return type == Type::Object; }
bool Json::is_array() const { return type == Type::Array; }
bool Json::is_string() const { return type == Type::String; }

const Json* Json::find(const std::string& key) const {
    if (type != Type::Object) {
        return nullptr;
    }
    auto it = object.find(key);
    return it == object.end() ? nullptr : &it->second;
}

std::string Json::get_string(const std::string& key, const std::string& fallback) const {
    const Json* value = find(key);
    return value != nullptr && value->is_string() ? value->string : fallback;
}

double Json::get_number(const std::string& key, double fallback) const {
    const Json* value = find(key);
    if (value == nullptr) {
        return fallback;
    }
    if (value->type == Type::Number) {
        return value->number;
    }
    if (value->is_string()) {
        char* end = nullptr;
        double parsed = std::strtod(value->string.c_str(), &end);
        if (end != value->string.c_str()) {
            return parsed;
        }
    }
    return fallback;
}

bool Json::get_bool(const std::string& key, bool fallback) const {
    const Json* value = find(key);
    return value != nullptr && value->type == Type::Bool ? value->boolean : fallback;
}

const Json& Json::at(size_t index) const {
    if (type != Type::Array || index >= array.size()) {
        throw std::runtime_error("JSON array index out of range");
    }
    return array[index];
}

size_t Json::size() const {
    if (type == Type::Array) {
        return array.size();
    }
    if (type == Type::Object) {
        return object.size();
    }
    return 0;
}

Json json_array(std::initializer_list<Json> values) {
    Json result;
    result.type = Json::Type::Array;
    result.array = values;
    return result;
}

Json json_object(std::initializer_list<std::pair<const std::string, Json>> values) {
    Json result;
    result.type = Json::Type::Object;
    result.object = values;
    return result;
}

}
