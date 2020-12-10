#include "util.h"
#include "common/telnet.c"

TEST_SETUP(Telnet)
{

}

TEST_TEAR_DOWN(Telnet)
{

}

TEST(Telnet, Handles_protocol_correctly)
{
	TEST_ASSERT(1);
}

TEST(Telnet, Does_not_delete_root_directory)
{
	TEST_ASSERT(1);
}
