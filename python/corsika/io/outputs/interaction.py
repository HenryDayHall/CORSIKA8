"""
 Read data written by InteractionWriter.

 (c) Copyright 2024 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the 3-clause BSD license.
 See file LICENSE for a full version of the license.
"""

import logging
import os.path as op
from typing import Any

import pyarrow.parquet as pq
import yaml

from ..converters import arrow_to_numpy
from .output import Output
from .primary import Particle


class FirstInteraction(Particle):
    """
    Simple class that holds the information of the first interaction
    This is a complete wrapper of the `Particle` class at the moment
    but is "new" so that it can be distinguished using `type(thing)`
    """

    def __init__(self, prop_dict: dict):
        """
        Parameters
        ----------
        prop_dict: dict
            the entry from the output yaml file for a single shower
        """
        Particle.__init__(self, prop_dict)

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        out_str = "FirstInteraction:\n"
        for key in self.__dict__.keys():
            out_str += f"\t{key}: {self.__dict__[key]}\n"
        return out_str

    @property
    def momentum(self) -> list:
        return [self.px, self.py, self.pz]


class Interactions(Output):
    """
    Reads the interactions and secondaries
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
            self.__data = pq.read_table(op.join(path, "interactions.parquet"))
            with open(op.join(path, "summary.yaml"), "r") as f:
                temp = yaml.load(f, Loader=yaml.Loader)
                self.__projectiles = [FirstInteraction(x) for x in temp.values()]
        except Exception as e:
            logging.getLogger("corsika").warn(
                f"An error occured loading an Interaction: {e}"
            )

    @property
    def projectiles(self) -> list:
        return self.__projectiles

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
        Load the particle data of the particle interactions.

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
                    f"Unknown format '{dtype}' for Interaction. "
                    "We currently only support ['arrow', 'pandas', 'numpy']."
                )
            )

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        return f"Interaction('{self.config['name']}')"
