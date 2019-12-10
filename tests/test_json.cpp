#include <gtest/gtest.h>
#include "json.h"

class test_json : public ::testing::Test {};

TEST_F(test_json, json_unmarshal) {
    std::string json_s = R"({"a":"str","b":15,"c":45.23,"d":true,"f":{"g":"str","h":false,"i":64.9}})";
//    auto json = new json::Json();
    auto json = new Json();
    json->unmarshal(json_s, 0);
    ASSERT_EQ(json->get_s("f.g", "null"), "str");
    ASSERT_EQ(json->get_i("b", 0), 15);
    ASSERT_EQ(json->get_f("f.i", 0), 64.9);
    ASSERT_EQ(json->get_b("d", false), true);
}

TEST_F(test_json, json_marshal) {
    std::string json_s = R"({"a":"str","b":15,"c":45.23,"d":true,"f":{"g":"str","i":64.9,"h":false}})";
//    auto json = new json::Json();
    auto json = new Json();
    json->unmarshal(json_s, 0);
    ASSERT_EQ(json->marshal(), json_s);
}
