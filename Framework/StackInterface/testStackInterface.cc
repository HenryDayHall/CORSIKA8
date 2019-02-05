
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/stack/SecondaryView.h>
#include <corsika/stack/Stack.h>

#include <boost/type_index.hpp>
#include <type_traits>
using boost::typeindex::type_id_with_cvr;

template <typename T>
struct ID {
  typedef T type;
};

#include <iomanip>
#include <iostream>
#include <vector>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::stack;
using namespace std;

// definition of stack-data object
class TestStackData {

public:
  // these functions are needed for the Stack interface
  void Init() {}
  void Dump() const {}
  void Clear() { fData.clear(); }
  unsigned int GetSize() const { return fData.size(); }
  unsigned int GetCapacity() const { return fData.size(); }
  void Copy(const int i1, const int i2) { fData[i2] = fData[i1]; }
  void Swap(const int i1, const int i2) {
    double tmp0 = fData[i1];
    fData[i1] = fData[i2];
    fData[i2] = tmp0;
  }

  // custom data access function
  void SetData(const int i, const double v) { fData[i] = v; }
  double GetData(const int i) const { return fData[i]; }

  // these functions are also needed by the Stack interface
  void IncrementSize() { fData.push_back(0.); }
  void DecrementSize() {
    if (fData.size() > 0) { fData.pop_back(); }
  }

  // custom private data section
private:
  std::vector<double> fData;
};

// defintion of a stack-readout object, the iteractor dereference
// operator will deliver access to these function
template <typename StackIteratorInterface>
class TestParticleInterface : public ParticleBase<StackIteratorInterface> {
  using ParticleBase<StackIteratorInterface>::GetStack;
  using ParticleBase<StackIteratorInterface>::GetStackData;
  using ParticleBase<StackIteratorInterface>::GetIndex;
  using ParticleBase<StackIteratorInterface>::GetIterator;

public:
  /* normally this function does not need to be specified here, it is
   part of ParticleBase<StackIteratorInterface>.

   However, when overloading this function again with a different
   parameter set, as here, seems to require also the declaration of
   the original function here...
  */

  // default version for particle-creation from input data
  void SetParticleData(const double v) { SetData(v); }
  void SetParticleData(TestParticleInterface<StackIteratorInterface>& /*parent*/,
                       const double v) {
    SetData(v);
  }
  /// alternative set-particle data for non-standard construction from different inputs
  void SetParticleData(const double v, const double p) { SetData(v + p); }
  void SetParticleData(TestParticleInterface<StackIteratorInterface>& /*parent*/,
                       const double v, const double p) {
    SetData(v + p);
  }

  void SetData(const double v) { GetStackData().SetData(GetIndex(), v); }
  double GetData() const { return GetStackData().GetData(GetIndex()); }
};

typedef Stack<TestStackData, TestParticleInterface> StackTest;
typedef StackTest::ParticleType Particle;

TEST_CASE("Stack", "[Stack]") {

  // helper function for sum over stack data
  auto sum = [](const StackTest& stack) {
    double v = 0;
    for (const auto& p : stack) v += p.GetData();
    return v;
  };

  SECTION("StackInterface") {

    // construct a valid Stack object
    StackTest s;
    s.Init();
    s.Clear();
    s.AddParticle(0.);
    s.Copy(s.cbegin(), s.begin());
    s.Swap(s.begin(), s.begin());
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("construct") {

    // construct a valid, empty Stack object
    StackTest s;
  }

  SECTION("write and read") {

    StackTest s;
    s.AddParticle(9.9);
    const double v = sum(s);
    REQUIRE(v == 9.9);
  }

  SECTION("delete from stack") {

    StackTest s;
    REQUIRE(s.GetSize() == 0);
    StackTest::StackIterator p = s.AddParticle(0.); // valid way to access particle data
    p.SetData(9.9);
    REQUIRE(s.GetSize() == 1);
    s.Delete(p);
    REQUIRE(s.GetSize() == 0);
  }

  SECTION("delete particle") {

    StackTest s;
    REQUIRE(s.GetSize() == 0);
    auto p =
        s.AddParticle(9.9); // also valid way to access particle data, identical to above
    REQUIRE(s.GetSize() == 1);
    p.Delete();
    REQUIRE(s.GetSize() == 0);
  }

  SECTION("create secondaries") {

    StackTest s;
    REQUIRE(s.GetSize() == 0);
    auto iter = s.AddParticle(9.9);
    Particle& p = *iter; // also this is valid to access particle data
    REQUIRE(s.GetSize() == 1);
    p.AddSecondary(4.4);
    REQUIRE(s.GetSize() == 2);
    p.AddSecondary(3.3, 2.2);
    REQUIRE(s.GetSize() == 3);
    double v = 0;
    for (auto& p : s) { v += p.GetData(); }
    REQUIRE(v == 9.9 + 4.4 + 3.3 + 2.2);
  }

  SECTION("get next particle") {
    StackTest s;
    REQUIRE(s.GetSize() == 0);
    s.AddParticle(9.9);
    s.AddParticle(8.8);
    auto particle = s.GetNextParticle(); // first particle
    REQUIRE(particle.GetData() == 8.8);

    particle.Delete();
    auto particle2 = s.GetNextParticle(); // first particle
    REQUIRE(particle2.GetData() == 9.9);
    particle2.Delete();

    REQUIRE(s.GetSize() == 0);
  }

  SECTION("secondary view") {
    StackTest s;
    REQUIRE(s.GetSize() == 0);
    s.AddParticle(9.9);
    s.AddParticle(8.8);
    const double sumS = 9.9 + 8.8;

    auto particle = s.GetNextParticle();

    typedef SecondaryView<TestStackData, TestParticleInterface> StackTestView;
    StackTestView v(particle);
    REQUIRE(v.GetSize() == 0);

    {
      auto proj = v.GetProjectile();
      REQUIRE(proj.GetData() == particle.GetData());
    }

    v.AddSecondary(4.4);
    v.AddSecondary(4.5);
    v.AddSecondary(4.6);

    REQUIRE(v.GetSize() == 3);
    REQUIRE(s.GetSize() == 5);
    REQUIRE(!v.IsEmpty());

    auto sumView = [](const StackTestView& stack) {
      double v = 0;
      for (const auto& p : stack) { v += p.GetData(); }
      return v;
    };

    REQUIRE(sum(s) == sumS + 4.4 + 4.5 + 4.6);
    REQUIRE(sumView(v) == 4.4 + 4.5 + 4.6);

    v.DeleteLast();
    REQUIRE(v.GetSize() == 2);
    REQUIRE(s.GetSize() == 4);

    REQUIRE(sum(s) == sumS + 4.4 + 4.5);
    REQUIRE(sumView(v) == 4.4 + 4.5);

    auto pDel = v.GetNextParticle();
    v.Delete(pDel);
    REQUIRE(v.GetSize() == 1);
    REQUIRE(s.GetSize() == 3);

    REQUIRE(sum(s) == sumS + 4.4 + 4.5 - pDel.GetData());
    REQUIRE(sumView(v) == 4.4 + 4.5 - pDel.GetData());

    v.Delete(v.GetNextParticle());
    REQUIRE(sum(s) == sumS);
    REQUIRE(sumView(v) == 0);
    REQUIRE(v.IsEmpty());

    {
      auto proj = v.GetProjectile();
      REQUIRE(proj.GetData() == particle.GetData());
    }
  }
}
