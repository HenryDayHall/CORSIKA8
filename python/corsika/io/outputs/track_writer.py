"""
 Read data written by TrackWriter

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import logging
import os.path as op
from typing import Any

import pyarrow.parquet as pq

from ..converters import arrow_to_numpy
from .output import Output


class TrackWriter(Output):
    """
    Read particle data from a TrackWriter
    """

    def __init__(self, path: str):
        """
        Load the particle data into a parquet table.

        Parameters
        ----------
        path: str
            The path to the directory containing this output.
        """
        super().__init__(path)

        # try and load our data
        try:
            self.__data = pq.read_table(op.join(path, "tracks.parquet"))
        except Exception as e:
            logging.getLogger("corsika").warn(
                f"An error occured loading a TrackWriter: {e}"
            )

    def is_good(self) -> bool:
        """
        Returns true if this output has been read successfully
        and has the correct files/state/etc.

        Returns
        -------
        bool:
            True if this is a good output.
        """
        return self.__data is not None

    def astype(self, dtype: str = "pandas", **kwargs: Any) -> Any:
        """
        Load the particle data from this track writer.

        All additional keyword arguments are passed to `parquet.read_table`

        Parameters
        ----------
        dtype: str
            The data format to return the data in (i.e. numpy, pandas, etc.)

        Returns
        -------
        Any:
            The return type of this method is determined by `dtype`.
        """
        if dtype == "arrow":
            return self.__data
        elif dtype == "pandas":
            return self.__data.to_pandas()
        elif dtype == "numpy":
            return arrow_to_numpy.convert_to_numpy(self.__data)
        else:
            raise ValueError(
                (
                    f"Unknown format '{dtype}' for TrackWriter. "
                    "We currently only support ['arrow', 'pandas', 'numpy']."
                )
            )

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        return f"TrackWriter('{self.config['name']}')"
