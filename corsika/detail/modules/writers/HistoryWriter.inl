/*
 * (c) Copyright 2024 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>

namespace corsika {

  template <typename TTracking, typename TOutput>
  inline HistoryWriter<TTracking, TOutput>::HistoryWriter(
      media::ShowerAxis const& axis, ObservationPlane<TTracking, TOutput> const& obsPlane)
      : obsPlane_(obsPlane)
      , showerAxis_(axis)
      , nextUnusedLabel_(1)
      , showerId_(0) {}

  template<typename T>
    struct TD;

  template <typename TTracking, typename TOutput>
  template <typename TStackView>
  inline void HistoryWriter<TTracking, TOutput>::doSecondaries(TStackView& vS) {
    // Dumps the stack to the output parquet stream
    // The primary and secondaries are all written along with the slant depth
    //
    auto primary = vS.getProjectile();
    auto const parent_id = primary.getLabel();
    //Only summarise if the nextUnusedIt counter is 1
    if (nextUnusedLabel_ == 1) {

      auto dX = showerAxis_.getProjectedX(primary.getPosition());
      CORSIKA_LOG_INFO("First interaction at dX {}", dX);
      CORSIKA_LOG_INFO("Primary: {}, E_kin {}", primary.getPID(),
                       primary.getKineticEnergy());

      Vector const displacement = primary.getPosition() - obsPlane_.getPlane().getCenter();
      auto const x = displacement.dot(obsPlane_.getXAxis());
      auto const y = displacement.dot(obsPlane_.getYAxis());
      auto const z = displacement.dot(obsPlane_.getPlane().getNormal());

      auto const px = primary.getMomentum().dot(obsPlane_.getXAxis());
      auto const py = primary.getMomentum().dot(obsPlane_.getYAxis());
      auto const pz = primary.getMomentum().dot(obsPlane_.getPlane().getNormal());

      summary_["shower_" + std::to_string(showerId_)]["pdg"] =
          static_cast<int>(get_PDG(primary.getPID()));
      summary_["shower_" + std::to_string(showerId_)]["kinetic_energy"] =
          primary.getKineticEnergy() / 1_GeV;
      summary_["shower_" + std::to_string(showerId_)]["x"] = x / 1_m;
      summary_["shower_" + std::to_string(showerId_)]["y"] = y / 1_m;
      summary_["shower_" + std::to_string(showerId_)]["z"] = z / 1_m;
      summary_["shower_" + std::to_string(showerId_)]["px"] =
          static_cast<double>(px / 1_GeV);
      summary_["shower_" + std::to_string(showerId_)]["py"] =
          static_cast<double>(py / 1_GeV);
      summary_["shower_" + std::to_string(showerId_)]["pz"] =
          static_cast<double>(pz / 1_GeV);
      summary_["shower_" + std::to_string(showerId_)]["time"] = primary.getTime() / 1_s;
      // Also stick it in the output, cos why not.
      *(output_.getWriter())
          << showerId_ << static_cast<int>(get_PDG(primary.getPID()))
          << static_cast<float>(px / 1_GeV)
          << static_cast<float>(py / 1_GeV)
          << static_cast<float>(pz / 1_GeV)
          << static_cast<float>(x / 1_m)
          << static_cast<float>(y / 1_m)
          << static_cast<float>(z / 1_m)
          << static_cast<float>(primary.getKineticEnergy() / 1_GeV)
          << static_cast<float>(primary.getTime() / 1_s)
          //<< static_cast<int>(particle.getEventId())
          << static_cast<int>(primary.getLabel())
          << static_cast<int>(0)
          << parquet::EndRow;
    }

    // Loop through secondaries
    auto particle = vS.begin();
    while (particle != vS.end()) {

      particle.setLabel(nextUnusedLabel_++);
      // Get the momentum of the secondary, write w.r.t. observation plane
      auto const p_2nd = particle.getMomentum();
      Vector const d_2nd = particle.getPosition() - obsPlane_.getPlane().getCenter();


      *(output_.getWriter())
          << showerId_ << static_cast<int>(get_PDG(particle.getPID()))
          << static_cast<float>(p_2nd.dot(obsPlane_.getXAxis()) / 1_GeV)
          << static_cast<float>(p_2nd.dot(obsPlane_.getYAxis()) / 1_GeV)
          << static_cast<float>(p_2nd.dot(obsPlane_.getPlane().getNormal()) / 1_GeV)
          << static_cast<float>(d_2nd.dot(obsPlane_.getXAxis()) / 1_m)
          << static_cast<float>(d_2nd.dot(obsPlane_.getYAxis()) / 1_m)
          << static_cast<float>(d_2nd.dot(obsPlane_.getPlane().getNormal()) / 1_m)
          << static_cast<float>(particle.getKineticEnergy() / 1_GeV)
          << static_cast<float>(particle.getTime() / 1_s)
          //<< static_cast<int>(particle.getEventId())
          << static_cast<int>(particle.getLabel())
          << static_cast<int>(parent_id)
          << parquet::EndRow;

      ++particle;
    }

  }

  template <typename TTracking, typename TOutput>
  inline void HistoryWriter<TTracking, TOutput>::startOfLibrary(
      boost::filesystem::path const& directory) {
    output_.initStreamer((directory / ("history.parquet")).string());

    // enable compression with the default level
    output_.enableCompression();

    output_.addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                     parquet::ConvertedType::INT_32);
    output_.addField("px", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("py", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("pz", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("x", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("y", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("z", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("kinetic_energy", parquet::Repetition::REQUIRED,
                     parquet::Type::FLOAT, parquet::ConvertedType::NONE);
    output_.addField("time", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("label", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                     parquet::ConvertedType::INT_32);
    output_.addField("parent_label", parquet::Repetition::REQUIRED,
                     parquet::Type::INT32, parquet::ConvertedType::INT_32);

    output_.buildStreamer();

    showerId_ = 0;
    nextUnusedLabel_ = 1;
    summary_ = YAML::Node();
  }

  template <typename TTracking, typename TOutput>
  inline void HistoryWriter<TTracking, TOutput>::startOfShower(
      unsigned int const showerId) {
    showerId_ = showerId;
    nextUnusedLabel_ = 1;
  }

  template <typename TTracking, typename TOutput>
  inline void HistoryWriter<TTracking, TOutput>::endOfShower(unsigned int const) {}

  template <typename TTracking, typename TOutput>
  inline void HistoryWriter<TTracking, TOutput>::endOfLibrary() {
    output_.closeStreamer();
  }

  template <typename TTracking, typename TOutput>
  inline YAML::Node HistoryWriter<TTracking, TOutput>::getConfig() const {
    YAML::Node node;
    node["type"] = "Historys";
    node["units"]["energy"] = "GeV";
    node["units"]["length"] = "m";
    node["units"]["time"] = "ns";
    node["units"]["grammage"] = "g/cm^2";
    return node;
  }

  template <typename TTracking, typename TOutput>
  inline YAML::Node HistoryWriter<TTracking, TOutput>::getSummary() const {
    return summary_;
  }

} // namespace corsika
