/**
  \file
  Test Bit functions

  \author Hans Dembinski
  \version $Id: testBit.cc 25126 2014-02-03 22:13:10Z darko $
  \date 27 Jan 2014

  \ingroup testing
*/

#include <corsika/utl/Test.h>
#include <tst/Verify.h>
#include <utl/Bit.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cstdio>
#include <iostream>
#include <bitset>

using namespace tst;
using namespace utl;
using namespace std;


/**
  \ingroup testing
*/
class TestBit : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TestBit);
  CPPUNIT_TEST(TestGet);
  CPPUNIT_TEST(TestSet);
  CPPUNIT_TEST(TestMask);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { }

  void tearDown() { }

  void
  TestGet()
  {
    const int size = sizeof(int)*8;
    const int bc2 = 12345;
    int b2 = bc2;
    bitset<size> b1(bc2);

    ostringstream out1;
    ostringstream out2;
    ostringstream out3;
    for (int i = 0; i < size; ++i) {
      out1 << (b1[i] ? '^' : '.');
      out2 << (AsBitArray(bc2)[i] ? '^' : '.');
      out3 << (AsBitArray(b2)[i] ? '^' : '.');
    }

    CPPUNIT_ASSERT(Verify<Equal>(out1.str(), out2.str()));
    CPPUNIT_ASSERT(Verify<Equal>(out1.str(), out3.str()));
  }

  void
  TestSet()
  {
    const int size = sizeof(int)*8;
    const int number = 12345;
    bitset<size> b1(number);
    int b2 = 11111;

    for (int i = 0; i < size; ++i)
      AsBitArray(b2)[i] = b1[i];

    CPPUNIT_ASSERT(Verify<Equal>(b2, number));
  }

  void
  TestMask()
  {
    const int n = (1 << 18) | (1 << 5);
    int m = 0;

    AsBitArray(m)[18] = true;
    AsBitArray(m)[5] = true;
    CPPUNIT_ASSERT(Verify<Equal>(n, m));

    for (unsigned int i = 0; i < 8*sizeof(int); ++i)
      AsBitArray(m)[i] = 0;
    CPPUNIT_ASSERT(Verify<Equal>(m, 0));

    m = 1;
    AsBitArray(m).Mask(n, true);
    CPPUNIT_ASSERT(Verify<Equal>(m, n+1));

    AsBitArray(m).Mask(n, false);
    CPPUNIT_ASSERT(Verify<Equal>(m, 1));
  }

};


CPPUNIT_TEST_SUITE_REGISTRATION(TestBit);
