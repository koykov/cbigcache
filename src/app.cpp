#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdlib.h>
#include <thread>
#include <helpers.h>
#include "debug.h"
#include "bigcache.h"
#include "types.h"
#include "json.h"

std::array<std::string, 7> data_pool = {
        R"({"firstName":"John","lastName":"Smith","isAlive":true,"age":27,"address":{"streetAddress":"21 2nd Street","city":"New York","state":"NY","postalCode":"10021-3100"},"phoneNumbers":[{"type":"home","number":"212 555-1234"},{"type":"office","number":"646 555-4567"},{"type":"mobile","number":"123 456-7890"}],"children":[],"spouse":null})",
        R"({"$schema":"http://json-schema.org/schema#","title":"Product","type":"object","required":["id","name","price"],"properties":{"id":{"type":"number","description":"Product identifier"},"name":{"type":"string","description":"Name of the product"},"price":{"type":"number","minimum":0},"tags":{"type":"array","items":{"type":"string"}},"stock":{"type":"object","properties":{"warehouse":{"type":"number"},"retail":{"type":"number"}}}}})",
        R"({"id":1,"name":"Foo","price":123,"tags":["Bar","Eek"],"stock":{"warehouse":300,"retail":20}})",
        R"({"first name":"John","last name":"Smith","age":25,"address":{"street address":"21 2nd Street","city":"New York","state":"NY","postal code":"10021"},"phone numbers":[{"type":"home","number":"212 555-1234"},{"type":"fax","number":"646 555-4567"}],"sex":{"type":"male"}})",
        R"({"fruit":"Apple","size":"Large","color":"Red"})",
        R"({"quiz":{"sport":{"q1":{"question":"Which one is correct team name in NBA?","options":["New York Bulls","Los Angeles Kings","Golden State Warriros","Huston Rocket"],"answer":"Huston Rocket"}},"maths":{"q1":{"question":"5 + 7 = ?","options":["10","11","12","13"],"answer":"12"},"q2":{"question":"12 - 8 = ?","options":["1","2","3","4"],"answer":"4"}}}})",
};

std::string rand_s(std::size_t length)
{
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += characters[distribution(generator)];
    }

    return random_string;
}

void call_bc(BigCache *bc) {
    int i = random() % 6;
    std::string data_s = data_pool[i];
    const byte *data_b = reinterpret_cast<const byte*>(data_s.c_str());

    auto key = rand_s(20);
    error err = bc->set(key, data_b);
    if (err != ERR_OK) {
        std::cout << "set err " <<  err << std::endl;
    }

    byte *data_recv = new byte[1024];
    err = bc->get(key, data_recv, 1024);
    if (err != ERR_OK) {
        std::cout << "get err " <<  err << std::endl;
    }
}

int main() {
//    std::string d0_s(R"({"common":{"id":1625602874113484440,"name":"a7S77X6EoeSwZ2FNhFG8","updateTime":1567795863},"matches":{"lists":{"1004959":1569977703,"1004965":1569977703,"1004966":1569977703,"1005375":1569977703,"1005429":1569977703,"1005787":1569979506,"1005793":1569979506,"1005794":1569979506,"1006203":1569979506,"1006240":1569979506,"1006426":1567809705,"1006427":1567809752,"1006429":1567809883,"10126":1567968544},"ssps":{"284":{"matchName":"37516376672","expireTime":1594821588},"59":{"matchName":"gY8qfgiQ","expireTime":1592908210}},"dsps":{"56":{"matchName":"37516376672","expireTime":1592904418}}},"impressions":{"ads":{"3043993":{"count":1,"expireTime":1567878629},"3064060":{"count":1,"expireTime":1567865794},"3100545":{"count":3,"expireTime":1567882268},"3107228":{"count":6,"expireTime":1567866915},"3135935":{"count":7,"expireTime":1567881351},"3142568":{"count":1,"expireTime":1567882258}}},"skips":{},"expireDays":2589343,"extSkips":{"extSkip":{"125":{}}}})");
//    const byte *d0 = reinterpret_cast<const byte*>(d0_s.c_str());
//
//    std::string d1_s(R"({"common":{"id":-7887523973621501495,"name":"pCvCHMq2PMiu9gJvkvBR","updateTime":1567668117},"matches":{"lists":{"1003561":1568077257},"ssps":{"10":{"matchName":"40df4c9c-9cc5-4db2-a567-8db78368fb06","expireTime":1593562121},"173":{"matchName":"MD7ttalfQ4K6E4IH3KAsUg","expireTime":1595787025},"243":{"matchName":"oguid","expireTime":1592804959},"46":{"matchName":"MD7ttalfQ4K6E4IH3KAsUg","expireTime":1595787025},"59":{"matchName":"kQEcr6i8","expireTime":1598771465},"82":{"matchName":"MD7ttalfQ4K6E4IH3KAsUg","expireTime":1595787023},"95":{"matchName":"5d4af16709ede61025786821","expireTime":1596295964}},"dsps":{"175":{"matchName":"d4866c3d11774afbb1f0e46d460f3d88","expireTime":1594241322},"177":{"matchName":"ZjBkN2IyNDM=","expireTime":1596285967},"56":{"matchName":"40768117479","expireTime":1592505577}}},"impressions":{},"skips":{},"expireDays":393029,"extSkips":{}})");
//    const byte *d1 = reinterpret_cast<const byte*>(d1_s.c_str());
//
//    BigCache *bc = new BigCache(R"({"shards_cnt":16,"max_size":0,"expire_ns":1000000000,"vacuum_ns":60000000000})");
//    bc->set("a7S77X6EoeSwZ2FNhFG8", d0);
//    bc->set("pCvCHMq2PMiu9gJvkvBR", d1);
//
//    byte *a0 = new byte[1024];
//    error e0 = bc->get("a7S77X6EoeSwZ2FNhFG8", a0, 1024);
//    std::cout << e0 << std::endl;
//
//    byte *a1 = new byte[1024];
//    error e1 = bc->get("pCvCHMq2PMiu9gJvkvBR", a1, 1024);
//    std::cout << e1 << std::endl;
//
//    std::this_thread::sleep_for(std::chrono::seconds(15));
//
//    delete bc;

    auto bc = new BigCache(R"({"verbose_lvl":2,"shards_cnt":1024,"force_set":true,"max_size":104857600,"expire_ns":10000000000})");

    for (int k=0; k<10; k++) {
        std::vector<std::thread> thr_pool{};
        for (int i = 0; i < 10000; i++) {
            thr_pool.emplace_back(std::thread(&call_bc, bc));
        }
        for (auto &thr : thr_pool) {
            thr.join();
        }
        thr_pool.clear();
    }

    std::cout << "stop" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(120));

    std::cout << "now: " << unix_time_now_ns() << std::endl;

    delete bc;

    return 0;
}
