
/**
  *@file ${obj.name}_UT.cpp
  *@brief Unit test for ${obj.name}.c
  *@author ${obj.user}
  *@date ${obj.gentime}
  */

#ifdef UNIT_TESTING_ENABLED


extern "C"{
#include "${obj.name}.c"
}


#include <gtest/gtest.h>


TEST(${obj.name}Test, test01)
{
    int expected = 0;
    int actual = 1;
    //TODO do something

    ASSERT_EQ(expected, actual+2);
}


#endif
