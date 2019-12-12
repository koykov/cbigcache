#ifndef CBIGCACHE_JSON_H
#define CBIGCACHE_JSON_H

#include <map>
#include <string>
#include <regex>
#include "types.h"

class Json {
private:
    /**
     * Storage for data of arbitrary JSON types (string, int64, float64, bool and object).
     */
    std::map<std::string, std::string> data_s;
    std::map<std::string, int64_t> data_i;
    std::map<std::string, double> data_f;
    std::map<std::string, bool> data_b;
    std::map<std::string, Json*> data_o;

public:
    /**
     * Internal counter of captured chars amount.
     */
    ulong captured_len = 0;

    explicit Json();
    ~Json();

    /**
     * Clear storage and free the memory.
     */
    void reset();

    /**
     * Parse JSON string and fill storage with the data.
     *
     * @param s
     * @param offset
     */
    void unmarshal(const std::string &s, ulong offset);

    /**
     * Marshal all data to JSON string.
     *
     * @return JSON string
     */
    std::string marshal();

    /**
     * Marshall all data to JSON string according format params.
     *
     * @param nl New line symbols.
     * @param t  Indent symbols.
     * @return JSON string
     */
    std::string marshal(const std::string &nl, const std::string &t);

    /**
     * Marshal all data to indented JSON string.
     *
     * @return Indented JSON string
     */
    std::string marshal_indent();

    /**
     * Get stored string value if key exists, <code>def</code> value otherwise.
     *
     * You may specify path of the keys like "key_root.key_intermediate.key_final".
     * So, for the given JSON object
     * <code>
     * {"foo":{"bar":{"f":"value"}}}
     * </code>
     * the path to retrieve value will be "foo.bar.f".
     * @param keys
     * @param def
     * @return string
     */
    std::string get_s(const std::string &keys, const std::string &def);

    /**
     * Get stored int value. See <code>Json::get_s()</code> for details.
     *
     * @see Json::get_s()
     * @param keys
     * @param def
     * @return int64
     */
    int64 get_i(const std::string &keys, const int64_t &def);

    /**
     * The same as Json::get_i() but takes default value if get zero.
     *
     * @see Json::get_i()
     * @return int64
     */
    int64 get_inz(const std::string &keys, const int64_t &def);

    /**
     * Get stored float value. See <code>Json::get_s()</code> for details.
     *
     * @see Json::get_s()
     * @param keys
     * @param def
     * @return double
     */
    float64 get_f(const std::string &keys, const double &def);

    /**
     * Get stored boolean value. See <code>Json::get_s()</code> for details.
     *
     * @see Json::get_s()
     * @param keys
     * @param def
     * @return bool
     */
    bool get_b(const std::string &keys, const bool &def);

    /**
     * Get stored Json object. See <code>Json::get_s()</code> for details.
     *
     * @see Json::get_s()
     * @param keys
     * @param def
     * @return Json or null
     */
    Json* get_o(const std::string &keys);
};

#endif //CBIGCACHE_JSON_H
