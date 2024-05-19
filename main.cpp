#include "public/DB.h"

int main() {
    AnilopDB::Settings settings;
    auto db = AnilopDB::DB::Open( {
            { "CAR", "car.db" },
            { "PRICE", "price.db" }
        }, settings
    );

    auto car = db->Find("CAR", "BMW X6");
    if (!car.has_value()) {
        db->Put("CAR", "BMW X6", "Black");
        db->Put("PRICE", "BMW X6", "100");
    }

    auto read_tx = db->newReadTx({ "CAR" });
    auto bmw_result = read_tx->Find("CAR", "BMW X7");
    auto mercedes_result = read_tx->Find("CAR", "Mercedes GL");
    // ...
    read_tx->commit();

    auto tx = db->newWriteTx({ "CAR", "PRICE" });
    try {
        tx->Remove("CAR", "BMW X6");
        tx->Put("CAR", "Mercedes CLS", "White");
        tx->Put("PRICE", "Mercedes CLS", "200");
        // ...
        throw std::runtime_error("Some runtime error");
    } catch (...) {
        tx->rollback();
    }
    tx->commit();
}
