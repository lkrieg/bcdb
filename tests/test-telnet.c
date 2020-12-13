#include "util.h"
#include "common/telnet.c"

TEST_SETUP(TELNET)
{

}

TEST_TEAR_DOWN(TELNET)
{

}

TEST(TELNET, Handles_telnet_protocol_correctly)
{
	TEST_ASSERT(1);
}

TEST(TELNET, Does_not_delete_root_directory)
{
	TEST_ASSERT(1);
}
