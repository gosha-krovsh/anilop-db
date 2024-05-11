#include "public/Table.h"

int main() {
    AnilopDB::Settings settings;
    auto table = AnilopDB::Table::Open("example.db", settings);

    auto car = table->Find("BMW");
    if (!car.has_value()) {
        table->Put("BMW", "X6");
    }

    auto read_tx = table->newReadTx();
    auto bmw_result = read_tx->Find("BMW");
    auto mercedes_result = read_tx->Find("Mercedes");
    // ...
    read_tx->commit();

    auto tx = table->newWriteTx();
    try {
        tx->Remove("BMW");
        tx->Put("Mercedes", "CLS");
        // ...
        throw std::runtime_error("Some runtime error");
    } catch (...) {
        tx->rollback();
    }
    tx->commit();
}