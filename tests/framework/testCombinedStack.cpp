/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define protected public // to also test the internal state of objects

#include <corsika/framework/stack/CombinedStack.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/framework/stack/Stack.hpp>

#include <testTestStack.h> // for testing: simple stack. This is a
// test-build, and inluce file is obtained from CMAKE_CURRENT_SOURCE_DIR

#include <iomanip>
#include <vector>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace std;

////////////////////////////////////////////////////////////
// first level test: combine two stacks:
//                   StackTest = (TestStackData + TestStackData2)

// definition of stack-data object
class TestStackData2 {

public:
  // these functions are needed for the Stack interface
  void Clear() { fData2.clear(); }
  unsigned int GetSize() const { return fData2.size(); }
  unsigned int GetCapacity() const { return fData2.size(); }
  void Copy(const int i1, const int i2) { fData2[i2] = fData2[i1]; }
  void Swap(const int i1, const int i2) {
    double tmp0 = fData2[i1];
    fData2[i1] = fData2[i2];
    fData2[i2] = tmp0;
  }

  // custom data access function
  void SetData2(const int i, const double v) { fData2[i] = v; }
  double GetData2(const int i) const { return fData2[i]; }

  // these functions are also needed by the Stack interface
  void IncrementSize() { fData2.push_back(0.); }
  void DecrementSize() {
    if (fData2.size() > 0) { fData2.pop_back(); }
  }

  // custom private data section
private:
  std::vector<double> fData2;
};

// defintion of a stack-readout object, the iteractor dereference
// operator will deliver access to these function
template <typename T>
class TestParticleInterface2 : public T {

public:
  using T::GetIndex;
  using T::GetStackData;
  using T::SetParticleData;

  // default version for particle-creation from input data
  void SetParticleData(const std::tuple<double> v = {0.}) { SetData2(std::get<0>(v)); }
  void SetParticleData(TestParticleInterface2<T>& parent,
                       const std::tuple<double> v = {0.}) {
    SetData2(parent.GetData2() + std::get<0>(v));
  }
  void SetData2(const double v) { GetStackData().SetData2(GetIndex(), v); }
  double GetData2() const { return GetStackData().GetData2(GetIndex()); }
};

// combined stack: StackTest = (TestStackData + TestStackData2)
template <typename StackIter>
using CombinedTestInterfaceType =
    corsika::CombinedParticleInterface<TestParticleInterface,
                                              TestParticleInterface2, StackIter>;

using StackTest = CombinedStack<TestStackData, TestStackData2, CombinedTestInterfaceType>;

TEST_CASE("Combined Stack", "[stack]") {

  // helper function for sum over stack data
  auto sum = [](const StackTest& stack) {
    double v = 0;
    for (const auto& p : stack) v += p.GetData();
    return v;
  };
  auto sum2 = [](const StackTest& stack) {
    double v = 0;
    for (const auto& p : stack) v += p.GetData2();
    return v;
  };

  SECTION("StackInterface") {

    // construct a valid Stack object
    StackTest s;
    s.Clear();
    s.AddParticle(std::tuple{0.});
    s.Copy(s.cbegin(), s.begin());
    s.Swap(s.begin(), s.begin());
    CHECK(s.getSize() == 1);
  }

  SECTION("construct") {

    // construct a valid, empty Stack object
    StackTest s;
  }

  SECTION("write and read") {

    StackTest s;
    s.AddParticle(std::tuple{9.9});
    CHECK(sum2(s) == 0.);
    CHECK(sum(s) == 9.9);
  }

  SECTION("delete from stack") {

    StackTest s;
    CHECK(s.getSize() == 0);
    StackTest::StackIterator p =
        s.AddParticle(std::tuple{0.}); // valid way to access particle data
    p.SetData(8.9);
    p.SetData2(3.);
    CHECK(sum2(s) == 3.);
    CHECK(sum(s) == 8.9);
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 1);
    s.Delete(p);
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
  }

  SECTION("delete particle") {

    StackTest s;
    CHECK(s.getSize() == 0);
    auto p = s.AddParticle(
        std::tuple{9.9}); // also valid way to access particle data, identical to above
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 1);
    p.Delete();
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
  }

  SECTION("create secondaries") {
    StackTest s;
    CHECK(s.getSize() == 0);
    auto iter = s.AddParticle(std::tuple{9.9});
    iter.SetData2(2);
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 1);
    iter.AddSecondary(std::tuple{4.4});
    CHECK(s.getSize() == 2);
    CHECK(s.getEntries() == 2);
    // p.AddSecondary(3.3, 2.2, 1.);
    // CHECK(s.getSize() == 3);
    double v = 0;
    for (const auto& i : s) {
      v += i.GetData();
      CHECK(i.GetData2() == 2);
    }
    CHECK(v == 9.9 + 4.4);
  }

  SECTION("get next particle") {
    StackTest s;
    CHECK(s.getSize() == 0);
    CHECK(s.getEntries() == 0);
    CHECK(s.IsEmpty());

    auto p1 = s.AddParticle(std::tuple{9.9});
    auto p2 = s.AddParticle(std::tuple{8.8});
    p1.SetData2(20.2);
    p2.SetData2(20.3);
    CHECK(s.getSize() == 2);
    CHECK(s.getEntries() == 2);
    CHECK(!s.IsEmpty());

    auto particle = s.GetNextParticle(); // first particle
    CHECK(particle.GetData() == 8.8);
    CHECK(particle.GetData2() == 20.3);

    particle.Delete(); // only marks (last) particle as "deleted"
    CHECK(s.getSize() == 2);
    CHECK(s.getEntries() == 1);
    CHECK(!s.IsEmpty());

    /*
      This following call to GetNextParticle will realize that the
      current last particle on the stack was marked "deleted" and will
      purge it: stack size is reduced by one.
     */
    auto particle2 = s.GetNextParticle(); // first particle
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 1);
    CHECK(!s.IsEmpty());
    CHECK(particle2.GetData() == 9.9);
    CHECK(particle2.GetData2() == 20.2);

    particle2.Delete(); // also mark this particle as "deleted"
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
    CHECK(s.IsEmpty());
  }
}

////////////////////////////////////////////////////////////
// next level: combine three stacks:
// combined stack: StackTest2 = ((TestStackData + TestStackData2) + TestStackData3)

// definition of stack-data object
class TestStackData3 {

public:
  // these functions are needed for the Stack interface
  void Clear() { fData3.clear(); }
  unsigned int getSize() const { return fData3.size(); }
  unsigned int GetCapacity() const { return fData3.size(); }
  void Copy(const int i1, const int i2) { fData3[i2] = fData3[i1]; }
  void Swap(const int i1, const int i2) {
    double tmp0 = fData3[i1];
    fData3[i1] = fData3[i2];
    fData3[i2] = tmp0;
  }

  // custom data access function
  void SetData3(const int i, const double v) { fData3[i] = v; }
  double GetData3(const int i) const { return fData3[i]; }

  // these functions are also needed by the Stack interface
  void IncrementSize() { fData3.push_back(0.); }
  void DecrementSize() {
    if (fData3.size() > 0) { fData3.pop_back(); }
  }

  // custom private data section
private:
  std::vector<double> fData3;
};

// ---------------------------------------
// defintion of a stack-readout object, the iteractor dereference
// operator will deliver access to these function
template <typename T>
class TestParticleInterface3 : public T {

public:
  using T::GetIndex;
  using T::GetStackData;
  using T::SetParticleData;

  // default version for particle-creation from input data
  void SetParticleData(const std::tuple<double> v = {0.}) { SetData3(std::get<0>(v)); }
  void SetParticleData(TestParticleInterface3<T>& parent,
                       const std::tuple<double> v = {0.}) {
    SetData3(parent.GetData3() + std::get<0>(v));
  }
  void SetData3(const double v) { GetStackData().SetData3(GetIndex(), v); }
  double GetData3() const { return GetStackData().GetData3(GetIndex()); }
};

// double combined stack:
// combined stack
template <typename StackIter>
using CombinedTestInterfaceType2 =
    corsika::CombinedParticleInterface<StackTest::MPIType, TestParticleInterface3,
                                              StackIter>;

using StackTest2 = CombinedStack<typename StackTest::StackImpl, TestStackData3,
                                 CombinedTestInterfaceType2>;

TEST_CASE("Combined Stack - multi", "[stack]") {

  SECTION("create secondaries") {

    StackTest2 s;
    CHECK(s.getSize() == 0);
    CHECK(s.IsEmpty()); // size = entries = 0

    // add new particle, only provide tuple data for StackTest
    auto p1 = s.AddParticle(std::tuple{9.9});
    // add new particle, provide tuple data for both StackTest and TestStackData3
    auto p2 = s.AddParticle(std::tuple{8.8}, std::tuple{0.1});

    CHECK(s.getSize() == 2);
    CHECK(!s.IsEmpty()); // size = entries = 2

    // examples to explicitly change data on stack
    p2.SetData2(0.1); // not clear why this is needed, need to check
                      // SetParticleData workflow for more complicated
                      // settings
    p1.SetData3(20.2);
    p2.SetData3(10.3);

    CHECK(p1.GetData() == 9.9);
    CHECK(p1.GetData2() == 0.);
    p1.SetData2(10.2);
    CHECK(p1.GetData2() == 10.2);
    CHECK(p1.GetData3() == 20.2);

    CHECK(p2.GetData() == 8.8);
    CHECK(p2.GetData2() == 0.1);
    CHECK(p2.GetData3() == 10.3);

    auto particle = s.GetNextParticle(); // first particle
    CHECK(particle.GetData() == 8.8);
    CHECK(particle.GetData2() == 0.1);
    CHECK(particle.GetData3() == 10.3);

    auto sec = particle.AddSecondary(std::tuple{4.4});
    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 3);
    CHECK(sec.GetData() == 4.4);
    CHECK(sec.GetData2() == 0.1);
    CHECK(sec.GetData3() == 10.3);

    sec.Delete(); // mark for deletion: size=3, entries=2
    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 2);
    CHECK(!s.IsEmpty());

    s.last().Delete(); // mark for deletion: size=3, entries=1
    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 1);
    CHECK(!s.IsEmpty());

    /*
       GetNextParticle will find two entries marked as "deleted" and
       will purge this from the end of the stack: size = 1
    */
    s.GetNextParticle().Delete(); // mark for deletion: size=3, entries=0
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
    CHECK(s.IsEmpty());
  }
}

////////////////////////////////////////////////////////////

// final level test, create SecondaryView on StackTest2

/*
  See Issue 161

  unfortunately clang does not support this in the same way (yet) as
  gcc, so we have to distinguish here. If clang cataches up, we could
  remove the clang branch here and also in corsika::Cascade. The gcc
  code is much more generic and universal.
 */
template <typename StackIter>
using CombinedTestInterfaceType2 =
    corsika::CombinedParticleInterface<StackTest::MPIType, TestParticleInterface3,
                                              StackIter>;

using StackTest2 = CombinedStack<typename StackTest::StackImpl, TestStackData3,
                                 CombinedTestInterfaceType2>;

#if defined(__clang__)
using StackTestView =
    SecondaryView<typename StackTest2::StackImpl, CombinedTestInterfaceType2>;
#elif defined(__GNUC__) || defined(__GNUG__)
using StackTestView = corsika::MakeView<StackTest2>::type;
#endif

using Particle2 = typename StackTest2::ParticleType;

TEST_CASE("Combined Stack - secondary view") {

  SECTION("create secondaries via secondaryview") {

    StackTest2 stack;
    auto particle = stack.AddParticle(std::tuple{9.9});
    StackTestView view(particle);

    auto projectile = view.GetProjectile();
    projectile.AddSecondary(std::tuple{8.8});

    CHECK(stack.getSize() == 2);
  }
}
