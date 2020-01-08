#pragma once
#ifndef ASUKA_JSON_HPP
#define ASUKA_JSON_HPP

#include <boost/variant.hpp>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cxx_version.hpp"

namespace Asuka
{

enum class JsonType : std::size_t
{
    null,
    boolean,
    number,
    string,
    array,
    object,
    error
};

struct error_type { };
constexpr error_type error_value;

class Json
{
public:
    using Null    = std::nullptr_t;
    using Bool    = bool;
    using Number  = double;
    using String  = std::string;
    using Array   = std::vector<Json>;
    using Object  = std::map<String, Json>;
    using Error   = error_type;

private:
    using JsonValue = boost::variant<Null, Bool, Number,
        String, Array, Object, Error>;
    
    JsonValue mValue;
public:
    Json() : mValue(nullptr) { }
    Json(Null) : mValue(nullptr) { }
    Json(bool b) : mValue(b) { }

    template <typename T, std::enable_if_t<
        std::is_arithmetic<T>::value, int> = 0>
    Json(T val) : mValue(static_cast<double>(val)) { }

    Json(std::string str) : mValue(std::move(str)) { }
    Json(const char* str) : mValue(std::string(str)) { }
    Json(Array arr) : mValue(std::move(arr)) { }
    Json(Object obj) : mValue(std::move(obj)) { }
    Json(Error) : mValue(error_value) { }

    JsonType get_type() const noexcept
    {
        return static_cast<JsonType>(mValue.which());
    }

    const char* type_name() const noexcept
    {
        JsonType type = get_type();
        switch (type)
        {
        case JsonType::null:
            return "null";
        case JsonType::boolean:
            return "boolean";
        case JsonType::number:
            return "number";
        case JsonType::string:
            return "string";
        case JsonType::array:
            return "array";
        case JsonType::object:
            return "object";
        default:
            return "error";
        }
    }

    template <typename T>
    T& get_value()
    {
        return boost::get<T>(mValue);
    }

    template <typename T>
    const T& get_value() const
    {
        return boost::get<T>(mValue);
    }

    std::size_t size() const
    {
        JsonType type = get_type();
        switch (type)
        {
        case JsonType::null:
        case JsonType::boolean:
        case JsonType::number:
        case JsonType::string:
            return 1;
        case JsonType::array:
            return boost::get<Array>(mValue).size();
        case JsonType::object:
            return boost::get<Object>(mValue).size();
        default:
            return 0;
        }
    }

    Json& operator[](std::size_t idx)
    {
        assert(is_array());
        return boost::get<Array>(mValue)[idx];
    }

    const Json& operator[](std::size_t idx) const
    {
        assert(is_array());
        return boost::get<Array>(mValue)[idx];
    }

    Json& operator[](const std::string& key)
    {
        assert(is_object());
        return boost::get<Object>(mValue)[key];
    }

    const Json& operator[](const std::string& key) const
    {
        assert(is_object());
        return const_cast<Object&>(boost::get<Object>(mValue))[key];
    }

    bool is_null()    const noexcept { return get_type() == JsonType::null;    }
    bool is_boolean() const noexcept { return get_type() == JsonType::boolean; }
    bool is_number()  const noexcept { return get_type() == JsonType::number;  }
    bool is_string()  const noexcept { return get_type() == JsonType::string;  }
    bool is_array()   const noexcept { return get_type() == JsonType::array;   }
    bool is_object()  const noexcept { return get_type() == JsonType::object;  }
    bool is_error()   const noexcept { return get_type() == JsonType::error;   }

private:
    // dump auxiliary function
    static void dump(Null, std::string& out) 
    {
        out += "null";
    }

    static void dump(Bool val, std::string& out) 
    {
        out += val ? "true" : "false";
    }

    static void dump(Number num, std::string& out) 
    {
        out += std::to_string(num);
    }

    static void dump(const String& str, std::string& out)
    {
        out += '"';
        for (char ch : str)
        {
            switch (ch)
            {
            case '\\':
                out += "\\\\";
                break;
            case '/':
                out += "\\/";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\t':
                out += "\\t";
                break;
            case '\r':
                out += "\\r";
                break;
            default:
                if (ch < 0x20)
                {
                    char buff[7];
                    sprintf(buff, "\\u%04X", ch);
                    out += buff;
                }
                else
                {
                    out += ch;
                }
                break;
            }
        }

        out += '"';
    }

    static void dump(const Array& arr, std::string& out)
    {
        bool first_time = true;
        out += '[';
        for (const auto& j : arr)
        {
            if (!first_time)
                out += ", ";
            j.dump(out);
            first_time = false;
        }

        out += ']';
    }

    static void dump(const Object& obj, std::string& out) 
    {
        bool first_time = true;
        out += '{';
        for (const auto& p : obj)
        {
            if (!first_time)
                out += ", ";
            dump(p.first, out);
            out += ": ";
            p.second.dump(out);
            first_time = false;
        }

        out += '}';
    }

private:
    // parse auxiliary class
    struct parser
    {
        const std::string& str;
        std::size_t idx;
        bool fail;

        parser(const std::string& s) : str(s), idx(0), fail(false) {}
        parser(std::string&&) = delete;
        parser(parser&&) = delete;

        bool is_digit0()
        {
            return str[idx] >= '0' && str[idx] <= '9';
        }

        bool is_digit1()
        {
            return str[idx] >= '1' && str[idx] <= '9';
        }

        Json parse_error()
        {
            fail = true;
            return Json{ error_value };
        }

        void parse_whitespace()
        {
            if (str[idx] == ' ' || str[idx] == '\t' || str[idx] == '\n' || str[idx] == '\r')
                ++idx;
        }

        char get_next_char()
        {
            parse_whitespace();

            if (idx == str.size())
            {
                fail = true;
                return '\0';
            }

            return str[idx++];
        }

        Json parse_null()
        {
            if (str.compare(idx, 3, "ull") == 0)
            {
                idx += 3;
                return Json{ nullptr };
            }
            return parse_error();
        }

        Json parse_boolean(const char* s, std::size_t count, bool val)
        {
            if (str.compare(idx, count, s) == 0)
            {
                idx += count;
                fail = false;
                return Json{ val };
            }
            return parse_error();
        }

        Json parse_number()
        {
            std::size_t begin = --idx;
            if (str[idx] == '-') ++idx;
            
            if (str[idx] == '0') ++idx;
            else
            {
                if (!is_digit1())
                    return parse_error();
                for (++idx; is_digit0(); ++idx);
            }
            
            if (str[idx] == '.')
            {
                ++idx;
                if (!is_digit0())
                    return parse_error();
                for (++idx; is_digit0(); ++idx);
            }

            if (str[idx] == 'e' || str[idx] == 'E')
            {
                ++idx;
                if (str[idx] == '+' || str[idx] == '-') 
                    ++idx;

                if (!is_digit0())
                    return parse_error();
                for (++idx; is_digit0(); ++idx);
            }
            
            errno = 0;
            double value = strtod(str.c_str() + begin, nullptr);
            if (errno == ERANGE && (value == HUGE_VAL || value == -HUGE_VAL))
                return parse_error();

            return Json{ value };
        }

        void encode_utf8(std::string& ret, std::uint32_t u)
        {
            if (u < 0x80)
            {
                ret += static_cast<char>(u);
            }
            else if (u < 0x800)
            {
                ret += static_cast<char>(0xC0 | ((u >> 6)  & 0x1F));
                ret += static_cast<char>(0x80 | ( u        & 0x3F));
            }
            else if (u < 0x10000)
            {
                ret += static_cast<char>(0xE0 | ((u >> 12) & 0x0F));
                ret += static_cast<char>(0x80 | ((u >> 6)  & 0x3F));
                ret += static_cast<char>(0x80 | ( u        & 0x3F));
            }
            else
            {
                ret += static_cast<char>(0xF0 | ((u >> 18) & 0x07));
                ret += static_cast<char>(0x80 | ((u >> 12) & 0x3F));
                ret += static_cast<char>(0x80 | ((u >> 6)  & 0x3F));
                ret += static_cast<char>(0x80 | ( u        & 0x3F));
            }
        }

        bool parse_hex4(std::uint32_t& u)
        {
            for (int i = 0; i < 4; ++i)
            {
                char ch = str[idx++];
                u <<= 4;
                if (ch >= '0' && ch <= '9')
                    u |= (ch - '0');
                else if (ch >= 'A' && ch <= 'F')
                    u |= (ch - ('A' - 10));
                else if (ch >= 'a' && ch <= 'f')
                    u |= (ch - ('a' - 10));
                else
                    return false;
            }

            return true;
        }

        Json parse_string()
        {
            String ret;

            while (true)
            {
                if (idx == str.size())
                    return parse_error();

                char ch = str[idx++];

                if (ch == '\"')
                {
                    break;
                }
                else if (ch == '\\')
                {
                    ch = str[idx++];
                    switch (ch)
                    {
                    case '\"': ret += '\"'; break;
                    case '\\': ret += '\\'; break;
                    case '/' : ret += '/' ; break;
                    case 'b' : ret += '\b'; break;
                    case 'f' : ret += '\f'; break;
                    case 'n' : ret += '\n'; break;
                    case 'r' : ret += '\r'; break;
                    case 't' : ret += '\t'; break;
                    case 'u' :
                    {
                        std::uint32_t high = 0;
                        if (!parse_hex4(high))
                            return parse_error();

                        if (high >= 0xD800 && high <= 0xDBFF)   // is high surrogate
                        {
                            if (str[idx++] != '\\')         
                                return parse_error();
                            if (str[idx++] != 'u')
                                return parse_error();

                            std::uint32_t low = 0;
                            if (!parse_hex4(low))
                                return parse_error();

                            if (low < 0xDC00 || low > 0xDFFF)   // is not low surrogate
                                return parse_error();

                            high = 0x10000 + (((high - 0xD800) << 10) | (low - 0xDC00));
                        }
                        encode_utf8(ret, high);
                        break;
                    }
                    default:
                        return parse_error();
                        break;
                    }
                }
                else
                {
                    if (ch < 0x20)
                        return parse_error();
                    ret += ch;
                }
            }

            return Json{ ret };
        }

        Json parse_array()
        {
            Array vec;
            char ch = get_next_char();

            if (ch == ']')
                return Json{ std::move(vec) };

            for (;;)
            {
                --idx;
                vec.push_back(parse_json());

                ch = get_next_char();

                if (ch == ']')
                    break;
                else if (ch != ',')
                    return parse_error();

                ch = get_next_char();
            }

            return Json{ std::move(vec) };
        }

        Json parse_object()
        {
            Object obj;
            char ch = get_next_char();

            if (ch == '}')
                return Json{ obj };

            for (;;)
            {
                if (ch != '"')
                    return parse_error();

                Json jstr = parse_string();
                
                ch = get_next_char();
                if (ch != ':')
                    return parse_error();

                obj[jstr.get_value<String>()] = parse_json();

                ch = get_next_char();

                if (ch == '}')
                    break;
                else if (ch != ',')
                    return parse_error();

                ch = get_next_char();
            }

            return Json{ std::move(obj) };
        }

        Json parse_json()
        {
            char ch = get_next_char();
            switch (ch)
            {
            case 'n':
                return parse_null();
            case 't':
                return parse_boolean("rue", 3, true);
            case 'f':
                return parse_boolean("alse", 4, false);
            case '-': 
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return parse_number();
            case '"':
                return parse_string();
            case '[':
                return parse_array();
            case '{':
                return parse_object();
            default:
                return Json{ error_value };
                break;
            }
        }
    };

public:
    std::string dump() const
    {
        std::string ret;
        dump(ret);
        return ret;
    }

    void dump(std::string& out) const
    {
        JsonType type = get_type();
        switch (type)
        {
        case JsonType::null:
            dump(boost::get<Null>(mValue), out);
            break;
        case JsonType::boolean:
            dump(boost::get<Bool>(mValue), out);
            break;
        case JsonType::number:
            dump(boost::get<Number>(mValue), out);
            break;
        case JsonType::string:
            dump(boost::get<String>(mValue), out);
            break;
        case JsonType::array:
            dump(boost::get<Array>(mValue), out);
            break;
        case JsonType::object:
            dump(boost::get<Object>(mValue), out);
            break;
        default:
            out = "parse error";
            break;
        }
    }

    void swap(Json& rhs) noexcept
    {
        using std::swap;
        swap(mValue, rhs.mValue);
    }

    static Json parse(const std::string& str)
    {
        parser p{ str };
        Json ret = p.parse_json();

        p.parse_whitespace();
        if (p.fail || p.idx != str.size())
            return Json{ error_value };

        return ret;
    }

    friend std::ostream& operator<<(std::ostream& os, const Json& j)
    {
        os << j.dump();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Json& j)
    {
        std::string str;
        is >> str;
        j = parse(str);
        return is;
    }
};

inline void swap(Json& lhs, Json& rhs) noexcept
{
    lhs.swap(rhs);
}

#ifdef ASUKACXX14
Json operator ""_json(const char* str, std::size_t n)
{
    return Json::parse(std::string(str, n));
}
#endif // ASUKACXX14

} // namespace Asuka

#endif 
