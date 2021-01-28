"""
 Read data written by ObservationPlane.

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import logging
import os.path as op
from typing import Any, Dict

import pyarrow.parquet as pq

from .output import Output


class ObservationPlane(Output):
    """
    Read particle data from an ObservationPlane.
    """

    def __init__(self, path: str):
        """
        Load the particle data into a parquet table.

        Parameters
        ----------
        path: str
            The path to the directory containing this output.
        """

        # load and store our path and config
        self.path = path
        self.__config = self.load_config(path)

        # try and load our data
        try:
            self.__data = pq.read_table(op.join(path, "particles.parquet"))
        except Exception as e:
            logging.getLogger("corsika").warn(
                f"An error occured loading an ObservationPlane: {e}"
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
        return self.__data is not None and self.__config is not None

    def astype(self, dtype: str = "parquet", **kwargs: Any) -> Any:
        """
        Load the particle data from this observation plane.

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
        if dtype == "parquet":
            return self.__data
        elif dtype == "pandas":
            return self.__data.to_pandas()
        else:
            raise ValueError(
                (
                    f"Unknown format '{dtype}' for ObservationPlane. "
                    "We currently only support ['parquet', 'pandas']."
                )
            )

    @property
    def config(self) -> Dict[str, Any]:
        """
        Return the config file for this output.

        Parameters
        ----------

        Returns
        -------
        Dict[str, any]
            The configuration file for this output.
        """
        return self.__config
