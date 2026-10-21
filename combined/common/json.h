#ifndef ECO_JSON_H
#define ECO_JSON_H

#include <string>
#include <cstdio>

namespace json {

inline std::string str(const std::string& s)
{
    std::string out = "\"";
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else if (c < 32)    out += ' ';
        else                out += (char)c;
    }
    return out + "\"";
}

inline std::string num(double v, int decimals = 0)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
    return buf;
}

inline std::string whole(double v) { return num((double)(long)(v + 0.5)); }

inline std::string boolean(bool b) { return b ? "true" : "false"; }

inline std::string ok(const std::string& body = "")
{
    return body.empty() ? "{\"ok\":true}" : "{\"ok\":true," + body + "}";
}

inline std::string error(const std::string& message)
{
    return "{\"ok\":false,\"error\":" + str(message) + "}";
}

}

#endif
