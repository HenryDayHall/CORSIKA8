/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <string>

// NOTE: the order of these includes is *important*
// you will get unhelpful compiler errors about unknown
// operator definitions if these are reordered
#include <parquet/stream_writer.h>
#include <parquet/arrow/schema.h>
#include <arrow/io/file.h>

namespace corsika::output {

  /**
   * This class automates the construction of simple tabular
   * Parquet files using the parquet::StreamWriter.
   */
  class ParquetStreamer final {

  public:
    ParquetStreamer() {}

    void Init(std::string const& filepath) {

      // open the file and connect it to our pointer
      PARQUET_ASSIGN_OR_THROW(outfile_, arrow::io::FileOutputStream::Open(filepath));

      // the default builder settings
      builder_.created_by("CORSIKA8");
    }

    /**
     * Add a field to this streamer.
     */
    template <typename... TArgs>
    void AddField(TArgs&&... args) {
      nodes_.push_back(parquet::schema::PrimitiveNode::Make(args...));
    }

    /**
     * Finalize the streamer construction.
     */
    void Build() {

      // build the top level schema
      auto schema = std::static_pointer_cast<parquet::schema::GroupNode>(
          parquet::schema::GroupNode::Make("schema", parquet::Repetition::REQUIRED,
                                           nodes_));

      // and build the writer
      writer_ = parquet::StreamWriter(
          parquet::ParquetFileWriter::Open(outfile_, schema, builder_.build()));
    }

    /**
     * Get a reference to the writer for this stream.
     */
    parquet::StreamWriter& GetWriter() { return writer_; }

    /**
     * Close the file.
     */
    void Close() { outfile_->Close(); }

    ///
  private:
    parquet::StreamWriter writer_;
    parquet::StreamWriter stream_;               ///< The stream writer to 'outfile'
    parquet::WriterProperties::Builder builder_; ///< The writer properties builder.
    parquet::schema::NodeVector nodes_;
    std::shared_ptr<arrow::io::FileOutputStream> outfile_; ///< The output file.

  }; // class ParquetHelper
} // namespace corsika::output
