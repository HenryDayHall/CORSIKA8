/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>
#include <typeinfo>

using namespace corsika;
using namespace std;

static const int nData = 10;

int globalCount = 0; // simple counter

int checkDecay = 0;    // use this as a bit field
int checkInteract = 0; // use this as a bit field
int checkSec = 0;      // use this as a bit field
int checkCont = 0;     // use this as a bit field

class ContinuousProcess1 : public ContinuousProcess<ContinuousProcess1> {
public:
  ContinuousProcess1(const int v)
      : v_(v) {

    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline ProcessReturn doContinuous(D& d, T&) const {
    cout << "ContinuousProcess1::DoContinuous" << endl;
    checkCont |= 1;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.933;
    return ProcessReturn::Ok;
  }

private:
  int v_ = 0;
};

class ContinuousProcess2 : public ContinuousProcess<ContinuousProcess2> {
public:
  ContinuousProcess2(const int v)
      : v_(v) {
    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline ProcessReturn doContinuous(D& d, T&) const {
    cout << "ContinuousProcess2::DoContinuous" << endl;
    checkCont |= 2;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.111;
    return ProcessReturn::Ok;
  }

private:
  int v_ = 0;
};

class ContinuousProcess3 : public ContinuousProcess<ContinuousProcess3> {
public:
  ContinuousProcess3(const int v)
      : v_(v) {
    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline ProcessReturn doContinuous(D& d, T&) const {
    cout << "ContinuousProcess3::DoContinuous" << endl;
    checkCont |= 4;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.333;
    return ProcessReturn::Ok;
  }

private:
  int v_ = 0;
};

class Process1 : public InteractionProcess<Process1> {
public:
  Process1(const int v)
      : v_(v) {
    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline void doInteraction(TView& v) const {
    checkInteract |= 1;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] += 1 + i;
  }

  template <typename TParticle>
  GrammageType getInteractionLength(TParticle&) const {
    return 10_g / square(1_cm);
  }

private:
  int v_;
};

class Process2 : public InteractionProcess<Process2> {
public:
  Process2(const int v)
      : v_(v) {
    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline void doInteraction(TView& v) const {
    checkInteract |= 2;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] /= 1.1;
    cout << "Process2::doInteraction" << endl;
  }
  template <typename Particle>
  GrammageType getInteractionLength(Particle&) const {
    cout << "Process2::GetInteractionLength" << endl;
    return 20_g / (1_cm * 1_cm);
  }

private:
  int v_ = 0;
};

class Process3 : public InteractionProcess<Process3> {
public:
  Process3(const int v)
      : v_(v) {
    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline void doInteraction(TView& v) const {
    checkInteract |= 4;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] *= 1.01;
    cout << "Process3::doInteraction" << endl;
  }
  template <typename Particle>
  GrammageType getInteractionLength(Particle&) const {
    cout << "Process3::GetInteractionLength" << endl;
    return 30_g / (1_cm * 1_cm);
  }

private:
  int v_ = 0;
};

class Process4 : public BaseProcess<Process4> {
public:
  Process4(const int v)
      : v_(v) {
    cout << "globalCount: " << globalCount << ", v_: " << v_ << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline ProcessReturn doContinuous(D& d, T&) const {
    std::cout << "Base::doContinuous" << std::endl;
    checkCont |= 8;
    for (int i = 0; i < nData; ++i) { d.data_[i] /= 1.2; }
    return ProcessReturn::Ok;
  }
  template <typename TView>
  void doInteraction(TView&) const {
    checkInteract |= 8;
  }

private:
  int v_ = 0;
};

class Decay1 : public DecayProcess<Decay1> {
public:
  Decay1(const int) {
    cout << "Decay1()" << endl;
    globalCount++;
  }

  template <typename Particle>
  TimeType getLifetime(Particle&) const {
    return 1_s;
  }
  template <typename TView>
  void doDecay(TView&) const {
    checkDecay |= 1;
  }
};

class Decay2 : public DecayProcess<Decay2> {
public:
  Decay2(const int) {
    cout << "Decay2()" << endl;
    globalCount++;
  }

  template <typename Particle>
  TimeType getLifetime(Particle&) const {
    return 2_s;
  }
  template <typename TView>
  void doDecay(TView&) const {
    checkDecay |= 2;
  }
};

class Stack1 : public StackProcess<Stack1> {
public:
  Stack1(const int n)
      : StackProcess(n) {}
  template <typename TStack>
  ProcessReturn doStack(TStack&) {
    count_++;
    return ProcessReturn::Ok;
  }
  int getCount() const { return count_; }

private:
  int count_ = 0;
};

struct DummyStack {};
struct DummyData {
  double data_[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
struct DummyTrajectory {};

struct DummyView {
  DummyView(DummyData& p)
      : p_(p) {}
  DummyData& p_;
  DummyData& parent() { return p_; }
};

TEST_CASE("Process Sequence General", "ProcessSequence") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$]: %v");

  SECTION("Check construction") {
    globalCount = 0;
    Process1 m1(0);
    CHECK(globalCount == 1);
    Process2 m2(1);
    CHECK(globalCount == 2);
    Process3 m3(2);
    CHECK(globalCount == 3);
    Process4 m4(3);
    CHECK(globalCount == 4);

    auto sequence1 = make_sequence(m1, m2, m3, m4);
    CHECK(is_process_sequence_v<decltype(sequence1)> == true);
    CHECK(is_process_sequence_v<decltype(m2)> == false);
    CHECK(is_switch_process_sequence_v<decltype(sequence1)> == false);
    CHECK(is_switch_process_sequence_v<decltype(m2)> == false);

    auto sequence2 = make_sequence(m1, m2, m3);
    CHECK(is_process_sequence_v<decltype(sequence2)> == true);

    auto sequence3 = make_sequence(m4);
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
  }

  SECTION("interaction length") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);

    DummyData particle;

    auto sequence2 = make_sequence(cp1, m2, m3);
    GrammageType const tot = sequence2.getInteractionLength(particle);
    InverseGrammageType const tot_inv = sequence2.getInverseInteractionLength(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;

    CHECK(tot / 1_g * square(1_cm) == 12);
    CHECK(tot_inv * 1_g / square(1_cm) == 1. / 12);
    globalCount = 0;
  }

  SECTION("lifetime") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);
    Decay1 d3(3);

    DummyData particle;

    auto sequence2 = make_sequence(cp1, m2, m3, d3);
    TimeType const tot = sequence2.getLifetime(particle);
    InverseTimeType const tot_inv = sequence2.getInverseLifetime(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;

    CHECK(tot / 1_s == 1);
    CHECK(tot_inv * 1_s == 1.);
    globalCount = 0;
  }

  SECTION("sectionTwo") {
    globalCount = 0;
    ContinuousProcess1 cp1(0); // += 0.933
    ContinuousProcess2 cp2(1); // += 0.111
    Process2 m2(2);            //  /= 1.1
    Process3 m3(3);            //  *= 1.01

    auto sequence2 = make_sequence(cp1, m2, m3, cp2);

    DummyData particle;
    DummyTrajectory track;

    cout << "-->init sequence2" << endl;
    globalCount = 0;
    cout << "-->docont" << endl;

    // validation data
    double test_data[nData] = {0};

    const int nLoop = 5;
    cout << "Running loop with n=" << nLoop << endl;
    for (int iLoop = 0; iLoop < nLoop; ++iLoop) {
      for (int i = 0; i < nData; ++i) { test_data[i] += 0.933 + 0.111; }
      sequence2.doContinuous(particle, track);
    }
    for (int i = 0; i < nData; i++) {
      cout << "data_[" << i << "]=" << particle.data_[i] << endl;
      CHECK(particle.data_[i] == Approx(test_data[i]).margin(1e-9));
    }
    cout << "done" << endl;
  }

  SECTION("StackProcess") {

    globalCount = 0;
    Stack1 s1(1);
    Stack1 s2(2);

    auto sequence1 = make_sequence(s1, s2);

    DummyStack stack;

    const int nLoop = 20;
    for (int i = 0; i < nLoop; ++i) { sequence1.doStack(stack); }

    CHECK(s1.getCount() == 20);
    CHECK(s2.getCount() == 10);

    ContinuousProcess2 cp2(1); // += 0.111
    Process2 m2(2);            //  /= 1.1
    auto sequence2 = make_sequence(cp2, m2);
    auto sequence3 = make_sequence(cp2, m2, s1);

    CHECK(is_process_sequence_v<decltype(sequence2)> == true);
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
    CHECK(contains_stack_process_v<decltype(sequence2)> == false);
    CHECK(contains_stack_process_v<decltype(sequence3)> == true);
  }
}

TEST_CASE("Switch Process Sequence", "ProcessSequence") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$]: %v");

  SECTION("Check construction") {

    struct TestSelect {
      SwitchResult operator()(const DummyData& p) const {
        std::cout << "TestSelect data=" << p.data_[0] << std::endl;
        if (p.data_[0] > 0) return SwitchResult::First;
        return SwitchResult::Second;
      }
    };
    TestSelect select1;

    auto sequence1 = make_sequence(Process1(0), ContinuousProcess2(0), Decay1(0));
    auto sequence2 = make_sequence(ContinuousProcess3(0), Process2(0), Decay2(0));

    auto sequence3 = make_sequence(ContinuousProcess1(0), Process3(0),
                                   SwitchProcessSequence(sequence1, sequence2, select1));

    auto sequence_alt = make_sequence(
        ContinuousProcess1(0), Process3(0),
        make_select(make_sequence(Process1(0), ContinuousProcess2(0), Decay1(0)),
                    make_sequence(ContinuousProcess3(0), Process2(0), Decay2(0)),
                    select1));

    // check that same process sequence can be build in different ways
    CHECK(typeid(sequence3) == typeid(sequence_alt));
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
    CHECK(is_process_sequence_v<decltype(
              SwitchProcessSequence(sequence1, sequence2, select1))> == true);

    DummyData particle;
    DummyTrajectory track;
    DummyView view(particle);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = 100; // data positive
    sequence3.doContinuous(particle, track);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0b011);
    CHECK(checkSec == 0);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence_alt.doContinuous(particle, track);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0b101);
    CHECK(checkSec == 0);

    // 1/(30g/cm2) is Process3
    InverseGrammageType lambda_select = .9 / 30. * square(1_cm) / 1_g;
    InverseTimeType time_select = 0.1 / second;

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = 100; // data positive
    sequence3.selectInteraction(view, lambda_select);
    sequence3.selectDecay(view, time_select);
    CHECK(checkInteract == 0b100); // this is Process3
    CHECK(checkDecay == 0b001);    // this is Decay1
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);
    lambda_select = 1.01 / 30. * square(1_cm) / 1_g;
    checkInteract = 0;
    sequence3.selectInteraction(view, lambda_select);
    CHECK(checkInteract == 0b001); // this is Process1

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence3.selectInteraction(view, lambda_select);
    sequence3.selectDecay(view, time_select);
    CHECK(checkInteract == 0b010); // this is Process2
    CHECK(checkDecay == 0b010);    // this is Decay2
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence3.doSecondaries(view);
    Stack1 stack(0);
    sequence3.doStack(stack);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);
  }
}

TEST_CASE("Continuous Process Indexing", "ProcessSequence") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$]: %v");

  SECTION("Check construction") {

  }
}
