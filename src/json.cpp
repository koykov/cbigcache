#include <cstdarg>
#include "json.h"
#include "helpers.h"
#include "types.h"

Json::Json() {
    this->data_s = std::map<std::string, std::string>{};
    this->data_i = std::map<std::string, int64>{};
    this->data_f = std::map<std::string, float64>{};
    this->data_b = std::map<std::string, bool>{};
    this->data_o = std::map<std::string, Json*>{};
}

Json::~Json() {
    this->reset();
}

void Json::reset() {
    this->captured_len = 0;
    this->data_s.clear();
    this->data_i.clear();
    this->data_f.clear();
    this->data_b.clear();
    for (auto &o : this->data_o) {
        delete o.second;
    }
    this->data_o.clear();
}

void Json::unmarshal(const std::string &s, ulong offset) {
    this->reset();

    bool inside = false;
    bool l = true;
    std::string left, right;

    const std::regex IS_F("[+-]?[0-9]+\\.[0-9]+");
    const std::regex IS_I("[+-]?[0-9]+");
    const std::regex IS_B("[true|false]+", std::regex_constants::icase);
    const std::regex IS_N("null", std::regex_constants::icase);

    for (ulong i = offset; i < s.length(); i++) {
        this->captured_len++;

        auto c = s[i];
        if (c == '"') {
            inside = !inside;
        }

        if (!inside) {
            if (c == ' ' || c == '\n' || c == '\t') {
                continue;
            }
            if (c == ':' && l) {
                left = trim(left, R"(" )");
                l = false;
                continue;
            }
            if (c == '{') {
                if (i == offset) {
                    continue;
                }
                Json *j = new Json();
                j->unmarshal(s, i);
                this->data_o[left] = j;
                i += j->captured_len;
                continue;
            }
            if (c == ',' || c == '}') {
                right = trim(right, " ");
                l = true;

                std::smatch m;
                if (std::regex_match(right, m, IS_F)) {
                    this->data_f[left] = atof64(right);
                } else if (std::regex_match(right, m, IS_I)) {
                    this->data_i[left] = atoi64(right);
                } else if (std::regex_match(right, m, IS_B)) {
                    this->data_b[left] = atob(right);
                } else if (std::regex_match(right, m, IS_N)) {
                    // Fallthrough this case.
                } else {
                    this->data_s[left] = trim(right, R"(")");
                }

                left = "";
                right = "";

                if (c == '}') {
                    return;
                }
                continue;
            }
        }

        if (l) {
            left += c;
        } else {
            right += c;
        }
    }
}

std::string Json::marshal(const std::string &nl, const std::string &t) {
    std::string sp = t.length() > 0 ? " " : "";
    std::ostringstream os, m_os;
    m_os << '{' + nl;
    for (auto &s : this->data_s) {
        os << t << '"' << s.first << R"(":)" << sp << '"' << s.second << R"(",)" << nl;
    }
    for (auto &i : this->data_i) {
        os << t << '"' << i.first << R"(":)" << sp << i.second << "," << nl;
    }
    for (auto &f : this->data_f) {
        os << t << '"' << f.first << R"(":)" << sp << f.second << "," << nl;
    }
    for (auto &b : this->data_b) {
        os << t << '"' << b.first << R"(":)" << sp << (b.second ? "true" : "false") << "," << nl;
    }
    for (auto &o : this->data_o) {
        os << t << '"' << o.first << R"(":)" << sp << o.second->marshal(nl, t.length() > 0 ? t + "\t" : "") << "," << nl;
    }
    std::string o = os.str();
    m_os << trim(o, "\n,");
    m_os << nl + t.substr(0, t.length()-1) + '}';
    return m_os.str();
}

std::string Json::marshal() {
    return this->marshal("", "");
}

std::string Json::marshal_indent() {
    return this->marshal("\n", "\t");
}

std::string Json::get_s(const std::string &keys, const std::string &def) {
    auto chunks = split(keys, '.');
    auto last = chunks.back();
    chunks.pop_back();
    auto o = this;
    for (auto &chunk : chunks) {
        o = o->get_o(chunk);
        if (o == nullptr) {
            return def;
        }
    }
    auto it = o->data_s.find(last);
    if ((it != o->data_s.end())) {
        return o->data_s[last];
    }
    return def;
}

int64 Json::get_i(const std::string &keys, const int64 &def) {
    auto chunks = split(keys, '.');
    auto last = chunks.back();
    chunks.pop_back();
    auto o = this;
    for (auto &chunk : chunks) {
        o = o->get_o(chunk);
        if (o == nullptr) {
            return def;
        }
    }
    auto it = o->data_i.find(last);
    if ((it != o->data_i.end())) {
        return o->data_i[last];
    }
    return def;
}

int64 Json::get_inz(const std::string &keys, const int64_t &def) {
    auto val = this->get_i(keys, 0);
    if (val == 0) {
        val = def;
    }
    return val;
}

float64 Json::get_f(const std::string &keys, const float64 &def) {
    auto chunks = split(keys, '.');
    auto last = chunks.back();
    chunks.pop_back();
    auto o = this;
    for (auto &chunk : chunks) {
        o = o->get_o(chunk);
        if (o == nullptr) {
            return def;
        }
    }
    auto it = o->data_f.find(last);
    if ((it != o->data_f.end())) {
        return o->data_f[last];
    }
    return def;
}

bool Json::get_b(const std::string &keys, const bool &def) {
    auto chunks = split(keys, '.');
    auto last = chunks.back();
    chunks.pop_back();
    auto o = this;
    for (auto &chunk : chunks) {
        o = o->get_o(chunk);
        if (o == nullptr) {
            return def;
        }
    }
    auto it = o->data_b.find(last);
    if ((it != o->data_b.end())) {
        return o->data_b[last];
    }
    return def;
}

Json* Json::get_o(const std::string &keys) {
    auto chunks = split(keys, '.');
    auto last = chunks.back();
    chunks.pop_back();
    auto o = this;
    for (auto &chunk : chunks) {
        o = o->get_o(chunk);
        if (o == nullptr) {
            return nullptr;
        }
    }
    auto it = o->data_o.find(last);
    if ((it != o->data_o.end())) {
        return o->data_o[last];
    }
    return nullptr;
}
