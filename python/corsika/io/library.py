"""
 This file allows for reading/working with C8 libraries.

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import logging
import os
import os.path as op
from typing import Any, Dict, List, Optional

import yaml

from . import outputs


class Library(object):
    """
    Represents a library ("run") of showers produced by C8.
    """

    def __init__(self, path: str):
        """

        Parameters
        ----------
        path: str
            The path to the directory containing the library.


        Raises
        ------
        ValueError
            If `path` does not contain a valid CORSIKA8 library.
        """

        # check that this is a valid library
        if not self.__valid_library(path):
            raise ValueError(f"'{path}' does not contain a valid CORSIKA8 library.")

        # store the top-level path
        self.path = path

        # load the config and summary files
        self.config = self.load_config(path)
        self.summary = self.load_summary(path)

        # build the list of outputs
        self.__outputs = self.__build_outputs(path)

    @property
    def names(self) -> List[str]:
        """
        Return the list of registered outputs.
        """
        return list(self.__outputs.keys())

    @property
    def modules(self) -> Dict[str, str]:
        """
        Return the list of registered outputs.
        """
        pass

    def get(self, name: str) -> Optional[outputs.Output]:
        """
        Return the output with a given name.
        """
        if name in self.__outputs:
            return self.__outputs[name]
        else:
            msg = f"Output with name '{name}' not available in this library."
            logging.getLogger("corsika").warn(msg)
            return None

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

    @staticmethod
    def __valid_library(path: str) -> bool:
        """
        Check if the library pointed to by 'path' is a valid C8 library.

        Parameters
        ----------
        path: str
            The path to the directory containing the library.

        Returns
        -------
        bool:
            True if this is a valid C8 library.

        """

        # check that the config file exists
        if not op.exists(op.join(path, "config.yaml")):
            return False

        # the config file exists, we load it
        config = Library.load_config(path)

        # and check that the config's "writer" key is correct
        return config["creator"] == "CORSIKA8"

    @staticmethod
    def __build_outputs(path: str) -> Dict[str, outputs.Output]:
        """
        Build the outputs contained in this library.

        This will print a warning message if a particular
        output is invalid but will continue to load additional
        outputs afterwards.

        Parameters
        ----------
        path: str
            The path to the directory containing this library.

        Returns
        -------
        Dict[str, Output]:
            A dictionary mapping names to initialized outputs.
        """

        # get a list of the subdirectories in the library
        _, dirs, _ = next(os.walk(path))

        # this is the dictionary where we store our components
        components: Dict[str, Any] = {}

        # loop over the subdirectories
        for subdir in dirs:

            # read the config file for this output
            config = Library.load_config(op.join(path, subdir))

            # the name keyword is our unique identifier
            name = config.get("name")

            # get the "type" keyword to identify this component
            out_type = config.get("type")

            # if `out_type` was None, this is an invalid output
            if out_type is None or name is None:
                msg = (
                    f"'{subdir}' does not contain a valid config."
                    "Missing 'type' or 'name' keyword."
                )
                logging.getLogger("corsika").warn(msg)
                continue  # skip to the next output, don't error

            # we now have a valid component type, get the corresponding
            # type from the proccesses subdirectory
            try:

                # instantiate the output and store it in our dict
                component = getattr(outputs, out_type)(op.join(path, subdir))

                # check if the read failed
                if not component.is_good():
                    msg = (
                        f"'{name}' encountered an error while reading. "
                        "This process will be not be loaded."
                    )
                    logging.getLogger("corsika").warn(msg)
                else:
                    components[name] = component

            except AttributeError as e:
                msg = (
                    f"Unable to instantiate an instance of '{out_type}' "
                    f"for a process called '{name}'"
                )
                logging.getLogger("corsika").warn(msg)
                logging.getLogger("corsika").warn(e)
                continue  # skip to the next output, don't error

        # and we are done building - return the constructed outputs
        return components
