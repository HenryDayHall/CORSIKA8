"""
 Read data written by a RadioProcess

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import logging
import os.path as op
from typing import Any

import numpy as np
import xarray as xr

from .output import Output


class RadioProcess(Output):
    """
    Read particle data from an RadioProcess.

    This *currently* can be used to read data written by ZHS
    or CoREAS implementations of the RadioProcess.
    """

    def __init__(self, path: str):
        """
        Initialize this radio process reader (and load the data).

        Since each antenna can have a different sample rate and duration,
        we can't load them into a shared array. Therefore, we load each
        of the antennas into an XArray dataset.

        Parameters
        ----------
        path: str
            The path to the directory containing this output.
        """
        super().__init__(path)

        # try and load our data
        try:
            self.__data = self.load_data(path)
        except Exception as e:
            logging.getLogger("corsika").warn(
                f"An error occured loading a RadioProcess: {e}"
            )

    def load_data(self, path: str) -> xr.Dataset:
        """
        Load the data associated with this radio process.

        Parameters
        ----------
        path: str
            The path to the directory containing this output.

        """

        # get the list of antenna names
        antennas = list(self.config["antennas"].keys())

        # if there are no antennas,
        if len(antennas) == 0:
            logging.warn(f"No antennas were found for {self.config['name']}")

        # we build up the XArray Dataset in this dictionary
        dataset = {}

        # loop over each of the antennas
        for iant, name in enumerate(antennas):

            # load the data file associated with this antenna
            try:
                data = np.load(f"{op.join(path, name)}.npz")
            except Exception as e:
                raise RuntimeError(
                    (
                        f"Unable to open file for antenna {name}"
                        f"in {self.config['name']} as {e}"
                    )
                )

            # if we get here, we have successfully loaded the antennas data file

            # extract the sample times (in ns)
            times = data["Time"]

            # calculate the number of showers for this antenna
            nshowers = len(list(data.keys())) - 1

            # check that we got some events
            if nshowers == 0:
                logging.warn(f"Antenna {name} contains data for 0 showers.")

            # create the array to store the waveforms for all the events
            waveforms = np.zeros((nshowers, *data["0"].shape), dtype=np.float32)

            # fill in the 'waveforms' array
            for iev in np.arange(nshowers):
                waveforms[iev, ...] = data[str(iev)]

            # create the  data array
            showers = xr.DataArray(
                waveforms,
                coords=(np.arange(nshowers), ["x", "y", "z"], times),  # type: ignore
                dims=["shower", "pol", "time"],
            )

            # save this data array
            dataset[name] = showers

        return xr.Dataset(dataset)

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

    def astype(self, dtype: str = "xarray", **kwargs: Any) -> Any:
        """
        Load the antenna data from this process.

        Parameters
        ----------
        dtype: str
            The data format to return the data in (i.e. numpy, pandas, etc.)

        Returns
        -------
        Any:
            The return type of this method is determined by `dtype`.
        """
        if dtype == "xarray":
            return self.__data
        else:
            raise ValueError(
                (
                    f"Unknown format '{dtype}' for RadioProcess. "
                    "We currently only support ['xarray']."
                )
            )

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        return f"RadioProcess('{self.config['name']}', '{self.config['algorithm']}')"
