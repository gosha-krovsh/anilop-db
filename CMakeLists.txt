cmake_minimum_required(VERSION 3.20)
project(Anilop)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -fsanitize=address,leak")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")

include(FetchContent)
FetchContent_Declare(
  gtest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(gtest)

include_directories(./)

add_executable(Anilop
    main.cpp

    settings/settings.h
    settings/settings.cpp
    
    exception/exception.h 
    exception/exception.cpp

    memory/type.h
    memory/memory.h
    memory/memory.cpp

    dal/dal.h
    dal/dal.cpp
    dal/page.h
    dal/page.cpp
    dal/item.h
    dal/item.cpp
    dal/node.h
    dal/node.cpp
    dal/freelist.h
    dal/freelist.cpp
    dal/meta.h
    dal/meta.cpp
    dal/serializable.h
    dal/log.cpp
    dal/log.h

    storage/storage.h
    storage/storage.cpp
    storage/log_storage.h
    storage/log_storage.cpp

    public/Table.cpp
    public/Table.h
    public/DB.cpp
    public/DB.h
    public/Transaction.cpp
    public/Transaction.h
    public/type.h
    public/type.cpp
    public/Settings.h

    storage/TransactionImpl.cpp
    storage/TransactionImpl.h

    dal/log_dal.cpp
    dal/log_dal.h
    dal/memory_log_dal.cpp
    dal/memory_log_dal.h
    dal/num_list.cpp
    dal/num_list.h

    memory/type.cpp
)

enable_testing()

add_executable(
    DALTest
    test/dal_help_test.cpp
    test/dal_test.cpp
    test/log_storage_test.cpp
    test/storage_test.cpp

    settings/settings.h
    settings/settings.cpp

    exception/exception.h
    exception/exception.cpp

    memory/type.h
    memory/type.cpp
    memory/memory.h
    memory/memory.cpp

    dal/dal.h
    dal/dal.cpp
    dal/page.h
    dal/page.cpp
    dal/item.h
    dal/item.cpp
    dal/node.h
    dal/node.cpp
    dal/freelist.h
    dal/freelist.cpp
    dal/meta.h
    dal/meta.cpp
    dal/serializable.h
    dal/log.cpp
    dal/log.h
    dal/log_dal.cpp
    dal/log_dal.h
    dal/memory_log_dal.cpp
    dal/memory_log_dal.h
    dal/num_list.cpp
    dal/num_list.h

    storage/log_storage.h
    storage/log_storage.cpp
    storage/storage.h
    storage/storage.cpp
)
target_link_libraries(
    DALTest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(DALTest)
