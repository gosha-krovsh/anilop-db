#include "public/DB.h"

int main() {
    AnilopDB::Settings settings;
    auto db = AnilopDB::DB::Open( {
            { "CAR", "car.db" },
            { "PRICE", "price.db" }
        }, settings
    );

    auto car = db->Find("CAR", "BMW");
    if (!car.has_value()) {
        db->Put("CAR", "BMW", "X6");
        db->Put("PRICE", "BMW", "100");
    }

    auto read_tx = db->newReadTx({ "CAR" });
    auto bmw_result = read_tx->Find("CAR", "BMW");
    auto mercedes_result = read_tx->Find("CAR", "Mercedes");
    // ...
    read_tx->commit();

    auto tx = db->newWriteTx({ "CAR", "PRICE" });
    try {
        tx->Remove("CAR", "BMW");
        tx->Put("CAR", "Mercedes", "CLS");
        tx->Put("PRICE", "Mercedes", "200");
        // ...
        throw std::runtime_error("Some runtime error");
    } catch (...) {
        tx->rollback();
    }
    tx->commit();
}
