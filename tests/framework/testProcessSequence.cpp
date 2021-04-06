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
#include <corsika/framework/process/ProcessTraits.hpp>
#include <corsika/framework/process/ContinuousProcessStepLength.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>
#include <typeinfo>

#include <boost/type_index.hpp>

/*
  Unit test for testing all Process types and their arrangement in
  containers ProcessSequence and SwitchProcessSequence
 */

using namespace corsika;
using namespace std;

static int const nData = 10;

// DummyNode is only needed for BoundaryCrossingProcess
struct DummyNode {
  DummyNode(int v)
      : data_(v) {}
  int data_ = 0;
};

// The stack is non-existent for this example
struct DummyStack {};

// our data object (particle) is a simple arrary of doubles
struct DummyData {
  double data_[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  typedef DummyNode node_type; // for BoundaryCrossingProcess
};

// there is no real trajectory/track
struct DummyTrajectory {};

// since there is no stack, there is also no view. This is a simplistic dummy object
// sufficient here.
struct DummyView {
  DummyView(DummyData& p)
      : p_(p) {}
  DummyData& p_;
  DummyData& parent() { return p_; }
};

int globalCount = 0; // simple counter

int checkDecay = 0;    // use this as a bit field
int checkInteract = 0; // use this as a bit field
int checkSec = 0;      // use this as a bit field
int checkCont = 0;     // use this as a bit field

class ContinuousProcess1 : public ContinuousProcess<ContinuousProcess1> {
public:
  ContinuousProcess1(int const v, LengthType const step)
      : v_(v)
      , step_(step) {

    CORSIKA_LOG_DEBUG(
        "globalCount: {} "
        ", v_: {} ",
        globalCount, v_);
    globalCount++;
  }

  void setStep(LengthType const v) { step_ = v; }

  template <typename D, typename T>
  ProcessReturn doContinuous(D& d, T&, bool flag) const {
    flag_ = flag;
    CORSIKA_LOG_TRACE("ContinuousProcess1::DoContinuous");
    checkCont |= 1;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.933;
    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrack>
  LengthType getMaxStepLength(TParticle&, TTrack&) {
    return step_;
  }

  bool getFlag() const { return flag_; }
  void resetFlag() { flag_ = false; }

private:
  int v_ = 0;
  LengthType step_ = 0_m;
  mutable bool flag_ = false;
};

class ContinuousProcess2 : public ContinuousProcess<ContinuousProcess2> {
public:
  ContinuousProcess2(int const v, LengthType const step)
      : v_(v)
      , step_(step) {
    CORSIKA_LOG_DEBUG(
        "globalCount: {}"
        ", v_: {}",
        globalCount, v_);
    globalCount++;
  }

  void setStep(LengthType const v) { step_ = v; }

  template <typename D, typename T>
  ProcessReturn doContinuous(D& d, T&, bool const flag) const {
    flag_ = flag;
    CORSIKA_LOG_DEBUG("ContinuousProcess2::DoContinuous");
    checkCont |= 2;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.111;
    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrack>
  LengthType getMaxStepLength(TParticle&, TTrack&) {
    return step_;
  }

  bool getFlag() const { return flag_; }
  void resetFlag() { flag_ = false; }

private:
  int v_ = 0;
  LengthType step_ = 0_m;
  mutable bool flag_ = false;
};

class ContinuousProcess3 : public ContinuousProcess<ContinuousProcess3> {
public:
  ContinuousProcess3(int const v, LengthType const step)
      : v_(v)
      , step_(step) {
    CORSIKA_LOG_DEBUG(
        "globalCount: {}"
        ", v_: {} ",
        globalCount, v_);
    globalCount++;
  }

  void setStep(LengthType const v) { step_ = v; }

  template <typename D, typename T>
  ProcessReturn doContinuous(D& d, T&, bool const flag) const {
    flag_ = flag;
    CORSIKA_LOG_DEBUG("ContinuousProcess3::DoContinuous");
    checkCont |= 4;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.333;
    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrack>
  LengthType getMaxStepLength(TParticle&, TTrack&) {
    return step_;
  }

  bool getFlag() const { return flag_; }
  void resetFlag() { flag_ = false; }

private:
  int v_ = 0;
  LengthType step_ = 0_m;
  mutable bool flag_ = false;
};

class Process1 : public InteractionProcess<Process1> {
public:
  Process1(int const v)
      : v_(v) {
    CORSIKA_LOG_DEBUG(
        "globalCount: {}"
        ", v_: {}",
        globalCount, v_);
    ;
    globalCount++;
  }

  template <typename TView>
  void doInteraction(TView& v) const {
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
  Process2(int const v)
      : v_(v) {
    CORSIKA_LOG_DEBUG(
        "globalCount: {}"
        ", v_: {}",
        globalCount, v_);
    globalCount++;
  }

  template <typename TView>
  void doInteraction(TView& v) const {
    checkInteract |= 2;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] /= 1.1;
    CORSIKA_LOG_DEBUG("Process2::doInteraction");
  }
  template <typename Particle>
  GrammageType getInteractionLength(Particle&) const {
    CORSIKA_LOG_DEBUG("Process2::GetInteractionLength");
    return 20_g / (1_cm * 1_cm);
  }

private:
  int v_ = 0;
};

class Process3 : public InteractionProcess<Process3> {
public:
  Process3(int const v)
      : v_(v) {
    CORSIKA_LOG_DEBUG(
        "globalCount: {}"
        ", v_: {}",
        globalCount, v_);
    globalCount++;
  }

  template <typename TView>
  void doInteraction(TView& v) const {
    checkInteract |= 4;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] *= 1.01;
    CORSIKA_LOG_DEBUG("Process3::doInteraction");
  }
  template <typename Particle>
  GrammageType getInteractionLength(Particle&) const {
    CORSIKA_LOG_DEBUG("Process3::GetInteractionLength");
    return 30_g / (1_cm * 1_cm);
  }

private:
  int v_ = 0;
};

class Process4 : public BaseProcess<Process4> {
public:
  Process4(int const v)
      : v_(v) {
    CORSIKA_LOG_DEBUG(
        "globalCount: {}"
        ", v_: {}",
        globalCount, v_);
    globalCount++;
  }

  template <typename D, typename T>
  ProcessReturn doContinuous(D& d, T&, bool const) const {
    CORSIKA_LOG_DEBUG("Base::doContinuous");
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
  Decay1(int const) {
    CORSIKA_LOG_DEBUG("Decay1()");
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
  Decay2(int const) {
    CORSIKA_LOG_DEBUG("Decay2()");
    globalCount++;
  }

  template <typename Particle>
  TimeType getLifetime(Particle&) const {
    return 2_s;
  }
  void doDecay(DummyView&) const { checkDecay |= 2; }
};

class Stack1 : public StackProcess<Stack1> {
public:
  Stack1(int const n)
      : StackProcess(n) {}
  template <typename TStack>
  void doStack(TStack const&) {
    count_++;
  }
  int getCount() const { return count_; }

private:
  int count_ = 0;
};

class Boundary1 : public BoundaryCrossingProcess<Boundary1> {
public:
  Boundary1(double const v = 1.0)
      : v_(v) {}

  template <typename Particle>
  ProcessReturn doBoundaryCrossing(Particle& p, typename Particle::node_type const& from,
                                   typename Particle::node_type const& to) {

    for (int i = 0; i < nData; ++i) { p.data_[i] += v_ * (from.data_ - to.data_); }
    return ProcessReturn::Ok;
  }

private:
  double v_ = 0.0;
};

TEST_CASE("ProcessSequence General", "ProcessSequence") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$]: %v");

  SECTION("BaseProcess") {

    Process1 m1(0);
    const Process4 m4(3);

    CHECK(is_process_v<Process1>);
    CHECK_FALSE(is_process_v<DummyData>);
    CHECK(is_process_v<decltype(m4)>);
    CHECK(is_process_v<decltype(Decay1(1))>);
    CHECK(is_process_v<decltype(ContinuousProcess3{3, 3_m})>);
  }

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
    CHECK(is_process_v<decltype(sequence1)>);
    CHECK(is_process_v<decltype(m2)>);
    CHECK(is_process_sequence_v<decltype(sequence1)>);
    CHECK_FALSE(is_process_sequence_v<decltype(m2)>);
    CHECK_FALSE(is_switch_process_sequence_v<decltype(sequence1)>);
    CHECK_FALSE(is_switch_process_sequence_v<decltype(m2)>);

    CHECK_FALSE(is_process_sequence_v<decltype(Decay1(7))>);
    CHECK_FALSE(is_switch_process_sequence_v<decltype(Decay1(7))>);

    auto sequence2 = make_sequence(m1, m2, m3);
    CHECK(is_process_sequence_v<decltype(sequence2)> == true);

    auto sequence3 = make_sequence(m4);
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
  }

  SECTION("interaction length") {
    globalCount = 0;
    ContinuousProcess1 cp1(0, 1_m);
    Process2 m2(1);
    Process3 m3(2);

    DummyData particle;

    auto sequence2 = make_sequence(cp1, m2, m3);
    GrammageType const tot = sequence2.getInteractionLength(particle);
    InverseGrammageType const tot_inv = sequence2.getInverseInteractionLength(particle);
    CORSIKA_LOG_DEBUG(
        "lambda_tot={}"
        "; lambda_tot_inv={}",
        tot, tot_inv);

    CHECK(tot / 1_g * square(1_cm) == 12);
    CHECK(tot_inv * 1_g / square(1_cm) == 1. / 12);
    globalCount = 0;
  }

  SECTION("lifetime") {
    globalCount = 0;
    ContinuousProcess1 cp1(0, 1_m);
    Process2 m2(1);
    Process3 m3(2);
    Decay1 d3(3);

    DummyData particle;

    auto sequence2 = make_sequence(cp1, m2, m3, d3);
    TimeType const tot = sequence2.getLifetime(particle);
    InverseTimeType const tot_inv = sequence2.getInverseLifetime(particle);
    CORSIKA_LOG_DEBUG(
        "lambda_tot={}"
        "; lambda_tot_inv={}",
        tot, tot_inv);

    CHECK(tot / 1_s == 1);
    CHECK(tot_inv * 1_s == 1.);
    globalCount = 0;
  }

  SECTION("ContinousProcess") {
    globalCount = 0;
    ContinuousProcess1 cp1(0, 1_m);   // += 0.933
    ContinuousProcess2 cp2(1, 1.1_m); // += 0.111
    Process2 m2(2);                   //  /= 1.1
    Process3 m3(3);                   //  *= 1.01

    auto sequence2 = make_sequence(cp1, m2, m3, cp2);

    std::cout << boost::typeindex::type_id<decltype(sequence2)>().pretty_name()
              << std::endl;

    DummyData particle;
    DummyTrajectory track;

    cp1.resetFlag();
    cp2.resetFlag();

    ContinuousProcessStepLength const step1 = sequence2.getMaxStepLength(particle, track);
    CHECK(LengthType(step1) == 1_m);
    sequence2.doContinuous(particle, track, step1);
    CHECK(cp1.getFlag());
    CHECK_FALSE(cp2.getFlag());
    CORSIKA_LOG_INFO("step1, l={}, i={}", LengthType(step1),
                     ContinuousProcessIndex(step1).getIndex());

    cp1.resetFlag();
    cp2.resetFlag();

    cp1.setStep(10_m);
    ContinuousProcessStepLength const step2 = sequence2.getMaxStepLength(particle, track);
    CHECK(LengthType(step2) == 1.1_m);
    CHECK(ContinuousProcessIndex(step1) != ContinuousProcessIndex(step2));
    sequence2.doContinuous(particle, track, step2);
    CHECK_FALSE(cp1.getFlag());
    CHECK(cp2.getFlag());
    CORSIKA_LOG_INFO("step2, l={}, i={}", LengthType(step2),
                     ContinuousProcessIndex(step2).getIndex());

    CORSIKA_LOG_DEBUG("-->init sequence2");
    globalCount = 0;
    CORSIKA_LOG_DEBUG("-->docont");

    // validation data
    double test_data[nData] = {0};

    // reset
    particle = DummyData();
    track = DummyTrajectory();

    int const nLoop = 5;
    CORSIKA_LOG_DEBUG("Running loop with n={}", nLoop);
    for (int iLoop = 0; iLoop < nLoop; ++iLoop) {
      for (int i = 0; i < nData; ++i) { test_data[i] += 0.933 + 0.111; }
      sequence2.doContinuous(particle, track, ContinuousProcessIndex(1));
    }
    for (int i = 0; i < nData; i++) {
      CORSIKA_LOG_DEBUG("data_[{}]={}", i, particle.data_[i]);
      CHECK(particle.data_[i] == Approx(test_data[i]).margin(1e-9));
    }
    CORSIKA_LOG_DEBUG("done");
  }

  SECTION("StackProcess") {

    globalCount = 0;
    Stack1 s1(1);
    Stack1 s2(2);

    auto sequence1 = make_sequence(s1, s2);

    DummyStack stack;

    int const nLoop = 20;
    for (int i = 0; i < nLoop; ++i) { sequence1.doStack(stack); }

    CHECK(s1.getCount() == 20);
    CHECK(s2.getCount() == 10);

    ContinuousProcess2 cp2(1, 2_m); // += 0.111
    Process2 m2(2);                 //  /= 1.1
    auto sequence2 = make_sequence(cp2, m2);
    auto sequence3 = make_sequence(cp2, m2, s1);

    CHECK(is_process_sequence_v<decltype(sequence2)> == true);
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
    CHECK(contains_stack_process_v<decltype(sequence2)> == false);
    CHECK(contains_stack_process_v<decltype(sequence3)> == true);
  }

  SECTION("BoundaryCrossingProcess") {

    globalCount = 0;
    Boundary1 b1;

    auto sequence1 = make_sequence(b1);

    DummyData particle;
    DummyNode node_from(5);
    DummyNode node_to(4);

    int const nLoop = 20;
    for (int i = 0; i < nLoop; ++i) {
      sequence1.doBoundaryCrossing(particle, node_from, node_to);
    }

    for (int i = 0; i < nData; i++) {
      CORSIKA_LOG_DEBUG("data_[{}]={}", i, particle.data_[i]);
      CHECK(particle.data_[i] == Approx(nLoop).margin(1e-9));
    }

    CHECK(is_process_sequence_v<decltype(sequence1)> == true);
    CHECK(contains_stack_process_v<decltype(sequence1)> == false);
    CHECK(count_processes<decltype(sequence1)>::count == 1);
  }
}

TEST_CASE("SwitchProcessSequence", "ProcessSequence") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$]: %v");

  /**
   * In this example switching is done only by "data_[0]>0", where
   * data in an arrray of doubles, DummyData.
   */

  struct SwitchSelect {
    SwitchResult operator()(DummyData const& p) const {
      if (p.data_[0] > 0) return SwitchResult::First;
      return SwitchResult::Second;
    }
  };
  SwitchSelect select1;

  auto cp1 = ContinuousProcess1(0, 1_m);
  auto cp2 = ContinuousProcess2(0, 2_m);
  auto cp3 = ContinuousProcess3(0, 3_m);

  auto sequence1 = make_sequence(Process1(0), cp2, Decay1(0), Boundary1(1.0));
  auto sequence2 = make_sequence(cp3, Process2(0), Boundary1(-1.0), Decay2(0));

  auto sequence3 = make_sequence(cp1, Process3(0),
                                 SwitchProcessSequence(sequence1, sequence2, select1));

  auto sequence4 =
      make_sequence(cp1, Boundary1(2.0), Process3(0),
                    SwitchProcessSequence(sequence1, Boundary1(-1.0), select1));

  SECTION("Check construction") {

    auto sequence_alt = make_sequence(
        cp1, Process3(0),
        make_select(make_sequence(Process1(0), cp2, Decay1(0), Boundary1(1.0)),
                    make_sequence(cp3, Process2(0), Boundary1(-1.0), Decay2(0)),
                    select1));

    auto switch_seq = SwitchProcessSequence(sequence1, sequence2, select1);
    CHECK(is_process_sequence_v<decltype(switch_seq)>);
    CHECK(is_switch_process_sequence_v<decltype(switch_seq)>);
    // CHECK(is_switch_process_sequence_v<decltype(&switch_seq)>);
    CHECK(is_switch_process_sequence_v<decltype(
              SwitchProcessSequence(sequence1, sequence2, select1))>);

    CHECK(is_process_sequence_v<decltype(sequence3)>);
    CHECK_FALSE(is_switch_process_sequence_v<decltype(sequence3)>);

    CHECK(is_process_sequence_v<decltype(
              SwitchProcessSequence(sequence1, sequence2, select1))>);
    CHECK(is_switch_process_sequence_v<decltype(
              SwitchProcessSequence(sequence1, sequence2, select1))>);

    // check that same process sequence can be build in different ways
    CHECK(typeid(sequence3) == typeid(sequence_alt));
    CHECK(is_process_sequence_v<decltype(sequence3)>);
    CHECK(is_process_sequence_v<decltype(
              SwitchProcessSequence(sequence1, sequence2, select1))>);
  }

  SECTION("Check interfaces") {

    DummyData particle;
    DummyTrajectory track;
    DummyView view(particle);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = 100; // data positive
    sequence3.doContinuous(particle, track, ContinuousProcessIndex(1));
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0b011);
    CHECK(checkSec == 0);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence3.doContinuous(particle, track, ContinuousProcessIndex(1));
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

    // check the SwitchProcessSequence where no process is selected in
    // selected branch (fallthrough)

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data positive
    sequence4.selectInteraction(view, lambda_select);
    sequence4.doSecondaries(view);
    sequence4.selectDecay(view, time_select);
    sequence4.doSecondaries(view);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);

    // check that large "select" value will correctly ignore the call
    lambda_select = 1e5 * square(1_cm) / 1_g;
    time_select = 1e5 / second;
    checkDecay = 0;
    checkInteract = 0;
    sequence3.selectInteraction(view, lambda_select);
    sequence3.selectDecay(view, time_select);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
  }

  SECTION("Check ContinuousProcesses in SwitchProcessSequence") {

    DummyData particle;
    DummyTrajectory track;

    particle.data_[0] =
        100; // data positive, selects particular branch on SwitchProcessSequence

    cp1.setStep(10_m);
    cp2.setStep(15_m);
    cp3.setStep(100_m);

    cp1.resetFlag();
    cp2.resetFlag();
    cp3.resetFlag();

    ContinuousProcessStepLength const step1 = sequence3.getMaxStepLength(particle, track);
    CHECK(LengthType(step1) == 10_m);
    sequence3.doContinuous(particle, track, step1);
    CHECK(cp1.getFlag());
    CHECK_FALSE(cp2.getFlag());
    CHECK_FALSE(cp3.getFlag());
    CORSIKA_LOG_INFO("step1, l={}, i={}", LengthType(step1),
                     ContinuousProcessIndex(step1).getIndex());

    particle.data_[0] =
        100; // data positive, selects particular branch on SwitchProcessSequence

    cp1.setStep(50_m);
    cp2.setStep(15_m);
    cp3.setStep(100_m);

    cp1.resetFlag();
    cp2.resetFlag();
    cp3.resetFlag();

    ContinuousProcessStepLength const step2 = sequence3.getMaxStepLength(particle, track);
    CHECK(LengthType(step2) == 15_m);
    sequence3.doContinuous(particle, track, step2);
    CHECK_FALSE(cp1.getFlag());
    CHECK(cp2.getFlag());
    CHECK_FALSE(cp3.getFlag());
    CORSIKA_LOG_INFO("step2, len_cont={}, indexLimit={} type={}", LengthType(step2),
                     ContinuousProcessIndex(step2).getIndex(),
                     boost::typeindex::type_id<decltype(sequence3)>().pretty_name());

    particle.data_[0] =
        -100; // data positive, selects particular branch on SwitchProcessSequence

    cp1.setStep(11_m);
    cp2.setStep(15_m);
    cp3.setStep(100_m);

    cp1.resetFlag();
    cp2.resetFlag();
    cp3.resetFlag();

    ContinuousProcessStepLength const step3 = sequence3.getMaxStepLength(particle, track);
    CHECK(LengthType(step3) == 11_m);
    sequence3.doContinuous(particle, track, step3);
    CHECK(cp1.getFlag());
    CHECK_FALSE(cp2.getFlag());
    CHECK_FALSE(cp3.getFlag());
    CORSIKA_LOG_INFO("step3, len_cont={}, indexLimit={} type={}", LengthType(step3),
                     ContinuousProcessIndex(step3).getIndex(),
                     boost::typeindex::type_id<decltype(sequence3)>().pretty_name());

    particle.data_[0] =
        -100; // data positive, selects particular branch on SwitchProcessSequence

    cp1.setStep(11_m);
    cp2.setStep(15_m);
    cp3.setStep(2_m);

    cp1.resetFlag();
    cp2.resetFlag();
    cp3.resetFlag();

    ContinuousProcessStepLength const step4 = sequence3.getMaxStepLength(particle, track);
    CHECK(LengthType(step4) == 2_m);
    sequence3.doContinuous(particle, track, step4);
    CHECK_FALSE(cp1.getFlag());
    CHECK_FALSE(cp2.getFlag());
    CHECK(cp3.getFlag());
    CORSIKA_LOG_INFO("step4, len_cont={}, indexLimit={} type={}", LengthType(step4),
                     ContinuousProcessIndex(step4).getIndex(),
                     boost::typeindex::type_id<decltype(sequence3)>().pretty_name());
  }

  SECTION("Check BoundaryCrossingProcess in SwitchProcessSequence") {

    DummyData particle;
    DummyNode node_from(1);
    DummyNode node_to(2);

    particle.data_[0] =
        100; // data positive, selects particular branch on SwitchProcessSequence

    sequence4.doBoundaryCrossing(particle, node_from, node_to);

    CHECK(particle.data_[0] == 97); // 100 - 2*1 - 1*1

    particle.data_[0] =
        -100; // data positive, selects particular branch on SwitchProcessSequence

    sequence4.doBoundaryCrossing(particle, node_from, node_to);
    CHECK(particle.data_[0] == -101); // -100 - 2*1 + 1*1
  }
}

TEST_CASE("ProcessSequence Indexing", "ProcessSequence") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$]: %v");

  SECTION("Indexing") {

    int const n0 = count_processes<Decay2>::count;
    int const n1 = count_processes<ContinuousProcess3>::count;
    int const n2 = count_processes<ContinuousProcess2,
                                   count_processes<ContinuousProcess3>::count>::count;
    int const n1_b =
        count_processes<Process2, count_processes<ContinuousProcess3>::count>::count;
    int const n1_c =
        count_processes<ContinuousProcess3, count_processes<Process2>::count>::count;
    int const n12 =
        count_processes<ContinuousProcess2,
                        count_processes<ContinuousProcess3, 10>::count>::count;
    int const n11_b =
        count_processes<Process1, count_processes<ContinuousProcess3, 10>::count>::count;
    int const n11_c =
        count_processes<ContinuousProcess3, count_processes<Process1, 10>::count>::count;

    CHECK(n0 == 1);
    CHECK(n1 == 1);
    CHECK(n1_b == 2);
    CHECK(n1_c == 2);
    CHECK(n2 == 2);
    CHECK(n11_b == 12);
    CHECK(n11_c == 12);
    CHECK(n12 == 12);

    std::cout << count_processes<ContinuousProcess3>::count << std::endl;
    std::cout << count_processes<Process3>::count << std::endl;

    struct SwitchSelect {
      SwitchResult operator()(DummyData const& p) const {
        std::cout << "SwitchSelect data=" << p.data_[0] << std::endl;
        if (p.data_[0] > 0) return SwitchResult::First;
        return SwitchResult::Second;
      }
    };

    auto sequence1 = make_sequence(Process1(0), ContinuousProcess2(0, 2_m), Decay1(0));
    auto sequence2 = make_sequence(ContinuousProcess3(0, 3_m), Process2(0), Decay2(0),
                                   ContinuousProcess1(0, 1_m));

    SwitchSelect select1;
    auto switch_seq = SwitchProcessSequence(sequence1, sequence2, select1);

    auto sequence3 = make_sequence(ContinuousProcess1(0, 1_m), Process3(0), switch_seq);
    auto sequence4 = make_sequence(ContinuousProcess1(0, 1_m), Process3(0),
                                   SwitchProcessSequence(sequence1, sequence2, select1));

    int const switch_seq_n = count_processes<decltype(switch_seq)>::count;
    int const sequence3_n = count_processes<decltype(sequence3)>::count;

    CHECK(decltype(sequence1)::getNumberOfProcesses() == 3);
    CHECK(count_processes<decltype(sequence1)>::count == 3);
    CHECK(count_processes<decltype(sequence2)>::count == 4);
    CHECK(switch_seq_n == 7);
    CHECK(sequence3_n == 9);
    CHECK(count_processes<decltype(sequence4)>::count == 9);

    std::cout << "switch_seq "
              << boost::typeindex::type_id<decltype(switch_seq)>().pretty_name()
              << std::endl;

    std::cout << "sequence3 "
              << boost::typeindex::type_id<decltype(sequence3)>().pretty_name()
              << std::endl;
  }
}
