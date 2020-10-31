/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
  class ParquetStreamer {

  public:
    ParquetStreamer() = default;

    /**
     * Initialize the streamer to write to a given file.
     */
    void InitStreamer(std::string const& filepath) {

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
      fields_.push_back(parquet::schema::PrimitiveNode::Make(args...));
    }

    /**
     * Finalize the streamer construction.
     */
    void BuildStreamer() {

      // build the top level schema
      schema_ = std::static_pointer_cast<parquet::schema::GroupNode>(
          parquet::schema::GroupNode::Make("schema", parquet::Repetition::REQUIRED,
                                           fields_));

      // and build the writer
      writer_ = std::make_shared<parquet::StreamWriter>(
          parquet::ParquetFileWriter::Open(outfile_, schema_, builder_.build()));
    }

    /**
     * Finish writing this stream.
     *
     */
    void CloseStreamer() {
      writer_.reset();
      outfile_->Close();
    }

  protected:
    std::shared_ptr<parquet::StreamWriter> writer_; ///< The stream writer to 'outfile'
    parquet::WriterProperties::Builder builder_;    ///< The writer properties builder.
    parquet::schema::NodeVector fields_;            ///< The fields in this file.
    std::shared_ptr<parquet::schema::GroupNode> schema_;   ///< The schema for this file.
    std::shared_ptr<arrow::io::FileOutputStream> outfile_; ///< The output file.

  }; // class ParquetHelper
} // namespace corsika::output
