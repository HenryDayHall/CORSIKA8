"""
 This file defines the API for all output readers.

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import os.path as op
from abc import ABC, abstractmethod
from typing import Any, Dict

import yaml


class Output(ABC):
    """
    This class defines the abstract interface for all classes
    that wish to provide reading support for CORSIKA8 outputs.
    """

    @abstractmethod
    def __init__(self, path: str):
        """
        __init__ must load the output files and check
        that it is valid.

        Parameters
        ----------
        path: str
            The path to the directory containing this output.
        """
        pass

    @abstractmethod
    def is_good(self) -> bool:
        """
        Returns true if this output has been read successfully
        and has the correct files/state/etc.

        Returns
        -------
        bool:
            True if this is a good output.
        """
        pass

    @abstractmethod
    def astype(self, dtype: str, **kwargs: Any) -> Any:
        """
        Return the data for this output in the data format given by 'dtype'

        Parameters
        ----------
        dtype: str
            The data format to return the data in (i.e. numpy, pandas, etc.)
        *args: Any
            Additional arguments can be accepted by the output.
        **kwargs: Any
            Additional keyword arguments can be accepted by the output.

        Returns
        -------
        Any:
            The return type of this method is determined by `dtype`.
        """
        pass

    @property
    @abstractmethod
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
        pass

    @staticmethod
    def load_config(path: str) -> Dict[str, Any]:
        """
        Load the top-level config from a given library path.

        Parameters
        ----------
        path: str
            The path to the directory containing the library.

        Returns
        -------
        dict:
            The config as a python dictionary.

        Raises
        ------
        FileNotFoundError
            If the config file cannot be found

        """
        with open(op.join(path, "config.yaml"), "r") as f:
            return yaml.load(f, Loader=yaml.Loader)

    @staticmethod
    def load_summary(path: str) -> Dict[str, Any]:
        """
        Load the top-level summary from a given library path.

        Parameters
        ----------
        path: str
            The path to the directory containing the library.

        Returns
        -------
        dict:
            The summary as a python dictionary.

        Raises
        ------
        FileNotFoundError
            If the summary file cannot be found

        """
        with open(op.join(path, "summary.yaml"), "r") as f:
            return yaml.load(f, Loader=yaml.Loader)
