#include <gtest/gtest.h>

#include "rom_tester_env.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new rom_tester_env(argv[1]));
    return RUN_ALL_TESTS();
}
