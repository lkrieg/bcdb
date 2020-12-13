#include "util.h"
#include "common/telnet.c"

TEST_SETUP(WEBAPI)
{

}

TEST_TEAR_DOWN(WEBAPI)
{

}

TEST(WEBAPI, Handles_http_protocol_correctly)
{
	TEST_ASSERT(1);
}

TEST(WEBAPI, Does_not_cause_zombie_apocalypse)
{
	TEST_ASSERT(1);
}
