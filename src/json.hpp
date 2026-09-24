// json.hpp - محلل ومولّد JSON صغير وخفيف (بدون أي اعتماديات خارجية)
#pragma once
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace mj {

class Value;
using VP = std::shared_ptr<Value>;

class Value {
public:
    enum Type { NUL, BOOL, NUM, STR, ARR, OBJ };
    Type type = NUL;
    bool b = false;
    double num = 0;
    std::string str;
    std::vector<VP> arr;
    std::vector<std::pair<std::string, VP>> obj;

    bool isNull() const { return type == NUL; }

    VP get(const std::string& key) const {
        if (type != OBJ) return nullptr;
        for (auto& kv : obj)
            if (kv.first == key) return kv.second;
        return nullptr;
    }
    VP at(size_t i) const {
        if (type != ARR || i >= arr.size()) return nullptr;
        return arr[i];
    }
    std::string asStr() const { return type == STR ? str : ""; }
};

inline void encodeUtf8(std::string& out, unsigned cp) {
    if (cp < 0x80) out += char(cp);
    else if (cp < 0x800) {
        out += char(0xC0 | (cp >> 6));
        out += char(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += char(0xE0 | (cp >> 12));
        out += char(0x80 | ((cp >> 6) & 0x3F));
        out += char(0x80 | (cp & 0x3F));
    } else {
        out += char(0xF0 | (cp >> 18));
        out += char(0x80 | ((cp >> 12) & 0x3F));
        out += char(0x80 | ((cp >> 6) & 0x3F));
        out += char(0x80 | (cp & 0x3F));
    }
}

class Parser {
    const char* p;
    const char* e;
    bool fail = false;

public:
    explicit Parser(const std::string& s) : p(s.data()), e(s.data() + s.size()) {}

    VP parse() {
        ws();
        VP v = value();
        return fail ? nullptr : v;
    }

private:
    void ws() {
        while (p < e && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
    }
    bool lit(const char* s) {
        size_t n = std::strlen(s);
        if ((size_t)(e - p) >= n && !std::strncmp(p, s, n)) { p += n; return true; }
        return false;
    }
    VP value() {
        ws();
        if (p >= e) { fail = true; return nullptr; }
        if (*p == '{') return object();
        if (*p == '[') return array();
        if (*p == '"') {
            auto v = std::make_shared<Value>();
            v->type = Value::STR;
            v->str = pstring();
            return v;
        }
        if (lit("true"))  { auto v = std::make_shared<Value>(); v->type = Value::BOOL; v->b = true;  return v; }
        if (lit("false")) { auto v = std::make_shared<Value>(); v->type = Value::BOOL; v->b = false; return v; }
        if (lit("null"))  { return std::make_shared<Value>(); }
        return number();
    }
    VP object() {
        auto v = std::make_shared<Value>();
        v->type = Value::OBJ;
        ++p; ws();
        if (p < e && *p == '}') { ++p; return v; }
        while (p < e) {
            ws();
            if (p >= e || *p != '"') { fail = true; return v; }
            std::string k = pstring();
            ws();
            if (p >= e || *p != ':') { fail = true; return v; }
            ++p;
            VP val = value();
            v->obj.emplace_back(std::move(k), val);
            ws();
            if (p < e && *p == ',') { ++p; continue; }
            if (p < e && *p == '}') { ++p; return v; }
            fail = true; return v;
        }
        fail = true; return v;
    }
    VP array() {
        auto v = std::make_shared<Value>();
        v->type = Value::ARR;
        ++p; ws();
        if (p < e && *p == ']') { ++p; return v; }
        while (p < e) {
            v->arr.push_back(value());
            ws();
            if (p < e && *p == ',') { ++p; continue; }
            if (p < e && *p == ']') { ++p; return v; }
            fail = true; return v;
        }
        fail = true; return v;
    }
    unsigned hex4() {
        unsigned r = 0;
        for (int i = 0; i < 4 && p < e; ++i, ++p) {
            r <<= 4;
            char c = *p;
            if (c >= '0' && c <= '9') r |= (unsigned)(c - '0');
            else if (c >= 'a' && c <= 'f') r |= (unsigned)(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') r |= (unsigned)(c - 'A' + 10);
            else { fail = true; return 0; }
        }
        return r;
    }
    std::string pstring() {
        std::string out;
        ++p; // skip "
        while (p < e && *p != '"') {
            if (*p == '\\') {
                ++p;
                if (p >= e) break;
                switch (*p) {
                    case 'n': out += '\n'; ++p; break;
                    case 't': out += '\t'; ++p; break;
                    case 'r': out += '\r'; ++p; break;
                    case 'b': out += '\b'; ++p; break;
                    case 'f': out += '\f'; ++p; break;
                    case '"': out += '"';  ++p; break;
                    case '\\': out += '\\'; ++p; break;
                    case '/': out += '/';  ++p; break;
                    case 'u': {
                        ++p;
                        unsigned cp = hex4();
                        if (cp >= 0xD800 && cp <= 0xDBFF && (size_t)(e - p) >= 6 &&
                            p[0] == '\\' && p[1] == 'u') {
                            p += 2;
                            unsigned lo = hex4();
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        }
                        encodeUtf8(out, cp);
                        break;
                    }
                    default: out += *p; ++p; break;
                }
            } else {
                out += *p; ++p;
            }
        }
        if (p < e) ++p; // skip closing "
        return out;
    }
    VP number() {
        char* endp = nullptr;
        double d = std::strtod(p, &endp);
        if (endp == p) { fail = true; return nullptr; }
        p = endp;
        auto v = std::make_shared<Value>();
        v->type = Value::NUM;
        v->num = d;
        return v;
    }
};

inline VP parse(const std::string& s) {
    Parser pr(s);
    return pr.parse();
}

// تهريب نص لوضعه داخل JSON
inline std::string escape(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n";  break;
            case '\r': o += "\\r";  break;
            case '\t': o += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\u%04x", c);
                    o += buf;
                } else {
                    o += char(c);
                }
        }
    }
    return o;
}

} // namespace mj
