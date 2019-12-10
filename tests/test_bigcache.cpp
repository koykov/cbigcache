#include <gtest/gtest.h>
#include <array>
#include "bigcache.h"

class test_bigcache : public ::testing::Test {

public:
    std::array<std::string, 7> data_pool = {
            R"({"firstName":"John","lastName":"Smith","isAlive":true,"age":27,"address":{"streetAddress":"21 2nd Street","city":"New York","state":"NY","postalCode":"10021-3100"},"phoneNumbers":[{"type":"home","number":"212 555-1234"},{"type":"office","number":"646 555-4567"},{"type":"mobile","number":"123 456-7890"}],"children":[],"spouse":null})",
            R"({"$schema":"http://json-schema.org/schema#","title":"Product","type":"object","required":["id","name","price"],"properties":{"id":{"type":"number","description":"Product identifier"},"name":{"type":"string","description":"Name of the product"},"price":{"type":"number","minimum":0},"tags":{"type":"array","items":{"type":"string"}},"stock":{"type":"object","properties":{"warehouse":{"type":"number"},"retail":{"type":"number"}}}}})",
            R"({"id":1,"name":"Foo","price":123,"tags":["Bar","Eek"],"stock":{"warehouse":300,"retail":20}})",
            R"({"first name":"John","last name":"Smith","age":25,"address":{"street address":"21 2nd Street","city":"New York","state":"NY","postal code":"10021"},"phone numbers":[{"type":"home","number":"212 555-1234"},{"type":"fax","number":"646 555-4567"}],"sex":{"type":"male"}})",
            R"({"fruit":"Apple","size":"Large","color":"Red"})",
            R"({"quiz":{"sport":{"q1":{"question":"Which one is correct team name in NBA?","options":["New York Bulls","Los Angeles Kings","Golden State Warriros","Huston Rocket"],"answer":"Huston Rocket"}},"maths":{"q1":{"question":"5 + 7 = ?","options":["10","11","12","13"],"answer":"12"},"q2":{"question":"12 - 8 = ?","options":["1","2","3","4"],"answer":"4"}}}})",
    };

    void call_bc(BigCache *bc) {
        std::string data_s = this->data_pool[rand() % 7];
        const byte *data_b = reinterpret_cast<const byte*>(data_s.c_str());

        auto key = this->rand_s(20);
        error err = bc->set(key, data_b);
        if (err != ERR_OK) {
            //
        }

        byte *data_recv = new byte[1024];
        err = bc->get(key, data_recv, 1024);
        if (err != ERR_OK) {
            //
        }
    }

    std::string rand_s(uint len)
    {
        srand(time(0));
        std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::string str_n;
        int pos;
        while(str_n.size() != len) {
            pos = ((rand() % (str.size() - 1)));
            str_n += str.substr(pos,1);
        }
        return str_n;
    }

};

//TEST_F(test_bigcache, bigcache_simple) {
//    std::string data_s = R"({"a":"str","b":15,"c":45.23,"d":true,"f":{"g":"str","i":64.9,"h":false}})";
//    const byte *data_b = reinterpret_cast<const byte*>(data_s.c_str());
//    auto bc = new BigCache(R"({"shards_cnt":8,"max_size":4194304,"expire_ns":500000})");
//    bc->set("simple_key", data_b);
//
//    byte *data_recv = new byte[1024];
//    auto err = bc->get("simple_key", data_recv, 1024);
//
//    delete bc;
//
//    ASSERT_EQ(err, ERR_OK);
//    ASSERT_EQ(*data_recv, *data_b);
//}

TEST_F(test_bigcache, gigcache_parallel) {
    auto bc = new BigCache(R"({"verbose_lvl":6,"shards_cnt":16,"force_set":true,"max_size":8388608,"expire_ns":10000000000})");

    std::vector<std::thread> thr_pool{};
    for (int i = 0; i < 5; i++) {
        thr_pool.emplace_back(std::thread(&test_bigcache::call_bc, this, bc));
    }

    for (auto &thr : thr_pool) {
        thr.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(15));

    delete bc;
}