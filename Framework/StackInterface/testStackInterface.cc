#include <corsika/stack/Stack.h>

#include <iomanip>
#include <iostream>
#include <vector>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::stack;
using namespace std;

// definition of stack-data object
class StackOneData {

public:
  // these functions are needed for the Stack interface
  void Init() {}
  void Clear() { fData.clear(); }
  int GetSize() const { return fData.size(); }
  int GetCapacity() const { return fData.size(); }
  void Copy(const int i1, const int i2) { fData[i2] = fData[i1]; }
  void Swap(const int i1, const int i2) {
    double tmp0 = fData[i1];
    fData[i1] = fData[i2];
    fData[i2] = tmp0;
  }

  // custom data access function
  void SetData(const int i, const double v) { fData[i] = v; }
  double GetData(const int i) const { return fData[i]; }

protected:
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
class ParticleInterface : public ParticleBase<StackIteratorInterface> {
  //  using ParticleBase<StackIteratorInterface>::Delete;
  using ParticleBase<StackIteratorInterface>::GetStackData;
  using ParticleBase<StackIteratorInterface>::GetIndex;

public:
  void SetData(const double v) { GetStackData().SetData(GetIndex(), v); }
  double GetData() const { return GetStackData().GetData(GetIndex()); }
};

TEST_CASE("Stack", "[Stack]") {

  SECTION("StackInterface") {

    // construct a valid Stack object
    typedef Stack<StackOneData, ParticleInterface> StackTest;
    StackTest s;
    s.Init();
    s.Clear();
    s.IncrementSize();
    s.Copy(0, 0);
    s.Swap(0, 0);
    s.GetCapacity();
    REQUIRE(s.GetSize() == 1);
    s.DecrementSize();
    REQUIRE(s.GetSize() == 0);
  }

  SECTION("write") {

    // construct a valid Stack object
    typedef Stack<StackOneData, ParticleInterface> StackTest;
    StackTest s;
  }

  SECTION("read") {

    typedef Stack<StackOneData, ParticleInterface> StackTest;
    StackTest s;
    s.NewParticle().SetData(9.9);
    cout << "kk" << endl;
    double v = 0;
    for (auto& p : s) {
      cout << typeid(p).name() << endl;
      v += p.GetData();
    }
    cout << "k222k" << endl;

    REQUIRE(v == 9.9);
  }

  SECTION("delete_stack") {

    typedef Stack<StackOneData, ParticleInterface> StackTest;
    StackTest s;
    auto p = s.NewParticle();
    p.SetData(9.9);
    s.Delete(p);
  }

  SECTION("delete_particle") {

    typedef Stack<StackOneData, ParticleInterface> StackTest;
    StackTest s;
    auto p = s.NewParticle();
    p.SetData(9.9);
    p.Delete();
  }
}
