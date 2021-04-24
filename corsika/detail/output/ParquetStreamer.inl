/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  ParquetStreamer::ParquetStreamer() {}

  void ParquetStreamer::initStreamer(std::string const& filepath) {

    // open the file and connect it to our pointer
    PARQUET_ASSIGN_OR_THROW(outfile_, arrow::io::FileOutputStream::Open(filepath));

    // the default builder settings
    builder_.created_by("CORSIKA8");

    // add run and event tags to the file
    addField("shower", parquet::Repetition::REQUIRED, parquet::Type::INT32,
             parquet::ConvertedType::INT_32);
  }

  template <typename... TArgs>
  void ParquetStreamer::addField(TArgs&&... args) {
    fields_.push_back(parquet::schema::PrimitiveNode::Make(args...));
  }

  void ParquetStreamer::enableCompression(int const /*level*/) {
    // builder_.compression(parquet::Compression::ZSTD);
    // builder_.compression_level(level);
  }

  void ParquetStreamer::buildStreamer() {

    // build the top level schema
    schema_ = std::static_pointer_cast<parquet::schema::GroupNode>(
        parquet::schema::GroupNode::Make("schema", parquet::Repetition::REQUIRED,
                                         fields_));

    // and build the writer
    writer_ = std::make_shared<parquet::StreamWriter>(
        parquet::ParquetFileWriter::Open(outfile_, schema_, builder_.build()));
  }

  void ParquetStreamer::closeStreamer() {
    writer_.reset();
    [[maybe_unused]] auto status = outfile_->Close();
  }

  std::shared_ptr<parquet::StreamWriter> ParquetStreamer::getWriter() { return writer_; }

} // namespace corsika
