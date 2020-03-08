
#include "gtest/gtest.h"
#include "test_helper.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    base_path = argv[1];
    return RUN_ALL_TESTS();
}
