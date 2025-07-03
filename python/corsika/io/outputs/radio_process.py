"""
Read data written by a RadioProcess

(c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

This software is distributed under the terms of the 3-clause BSD license.
See file LICENSE for a full version of the license.
"""

import logging
import os.path as op
from typing import Any

import pandas as pd
import pyarrow.parquet as pq

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

        We load each observer in a shared multidimensional pandas
        dataframe . Every observer can be accessed via the number
        of the shower and the observer name.

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

    def load_data(self, path: str) -> dict:
        """
        Load the data associated with this radio process.

        Parameters
        ----------
        path: str
            The path to the directory containing this output.

        """
        data = pq.read_table(op.join(path, "observers.parquet"))
        if not len(data.to_pandas()["shower"]):
            nshowers = 0
            observers = []
        else:
            nshowers = int(data.to_pandas()["shower"].iloc[-1] + 1)
            observers = list(self.config["observers"].keys())

        obs_nr = 0
        dictionary = {}
        dataset = {}
        # loop over all showers
        for i in range(nshowers):
            # loop over each of the observers
            for name in observers:
                sampling_period = self.config["observers"][name]["number of bins"]
                start = int(obs_nr * sampling_period)
                stop = int((obs_nr + 1) * sampling_period)
                observers_data = data[start:stop].to_pandas()
                times = observers_data["Time"]
                Ex = observers_data["Ex"]
                Ey = observers_data["Ey"]
                Ez = observers_data["Ez"]
                dictionary[name] = pd.DataFrame(
                    {"time": times, "Ex": Ex, "Ey": Ey, "Ez": Ez}
                )
                obs_nr += 1

            dataset[str(i)] = dictionary

        return dataset

    def is_good(self) -> bool:
        """
        Returns true if this output has been read successfully
        and has the correct files/state/etc.

        Returns
        -------
        bool:
            True if this is a good output.
        """
        return self.__data is not None and "observers" in self.config

    def astype(self, dtype: str = "pandas", **kwargs: Any) -> Any:
        """
        Load the observer data from this process.

        Parameters
        ----------
        dtype: str
            The data format to return the data in (i.e. numpy, pandas, etc.)

        Returns
        -------
        Any:
            The return type of this method is determined by `dtype`.
        """
        if dtype == "pandas":
            return self.__data
        else:
            raise ValueError(
                (
                    f"Unknown format '{dtype}' for RadioProcess. "
                    "We currently only support ['pandas']."
                )
            )

    def get_observers_list(self) -> list:
        """
        Return a list with the names of the observers of this RadioProcess.
        """
        if not self.is_good():
            return []
        observers = list(self.config["observers"].keys())
        return observers

    def get_observers(self) -> dict:
        """
        Return a pandas dataframe with all the information for the observers
        of this RadioProcess. This information is shower number, observers
        type, start time of detection, detection duration, number of bins,
        sampling frequency, location x, location y, location z.
        """
        observers = self.get_observers_list()
        dictionary = {}
        # loop over each of the observers
        for name in observers:
            type = self.config["observers"][name]["type"]
            start_time = self.config["observers"][name]["start time"]
            duration = self.config["observers"][name]["duration"]
            nr_bins = self.config["observers"][name]["number of bins"]
            sampling_frequency = self.config["observers"][name]["sampling frequency"]
            location = self.config["observers"][name]["location"]
            dictionary[name] = pd.DataFrame(
                {
                    "type": type,
                    "start time": start_time,
                    "duration": duration,
                    "number of bins": nr_bins,
                    "sampling frequency": sampling_frequency,
                    "x": [location[0]],
                    "y": [location[1]],
                    "z": [location[2]],
                }
            )

        return dictionary

    def get_units(self) -> dict:
        """
        Return the units of the observers of this RadioProcess.
        """
        return self.config["units"]

    def __repr__(self) -> str:
        """
        Return a string representation of this class.
        """
        return f"RadioProcess('{self.config['name']}', '{self.config['algorithm']}')"
