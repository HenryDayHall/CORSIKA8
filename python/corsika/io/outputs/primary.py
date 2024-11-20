"""
 Read data written by PrimaryWriter.

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""

import logging
import os.path as op
from typing import Any

import yaml

from .output import Output


class Particle(object):
    """
    A very basic class that is essentially a wrapper for the fields
    in the primary particle output file
    """

    def __init__(self, prop_dict: dict):
        """
        Parameters
        ----------
        prop_dict: dict
            the entry from the output yaml file for a single shower
        """

        # need to define for position/direction functions
        self.x = self.y = self.z = None
        self.nx = self.ny = self.nz = None
        self.px = self.py = self.pz = None

        # wrap the files of the dictionary so that there is a property
        # assigned to each value, i.e. print(my_primary.pdg)
        for key in prop_dict:
            self.__dict__[key] = prop_dict[key]

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        out_str = "PrimaryParticle:\n"
        for key in self.__dict__.keys():
            out_str += f"\t{key}: {self.__dict__[key]}\n"
        return out_str

    @property
    def position(self) -> list:
        return [self.x, self.y, self.z]

    @property
    def direction(self) -> list:
        return [self.nx, self.ny, self.nz]


class PrimaryParticle(Output):
    """
    Read particle data from the ParimaryWriter summary.yaml file
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
            with open(op.join(path, "summary.yaml"), "r") as f:
                self.__data = yaml.load(f, Loader=yaml.Loader)

            self.__primaries = [Particle(x) for x in self.__data.values()]

        except Exception as e:
            logging.getLogger("corsika").warn(
                f"An error occured loading a PrimaryParticle: {e}"
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

    def astype(self, dtype: str = "class", **kwargs: Any) -> Any:
        """
        Load the particle data from this particle cut instance.

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
        if dtype == "yaml":
            return self.__data
        elif dtype == "class":
            return self.__primaries
        else:
            raise ValueError(
                (
                    f"Unknown format '{dtype}' for PrimaryParticle. "
                    "We currently only support ['yaml', 'class']."
                )
            )

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        return f"PrimaryParticle('{self.config['name']}')"
