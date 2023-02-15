"""
 This file supports reading boost_histograms from CORSIKA8.

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""

import boost_histogram as bh
import numpy as np


def read_hist(filename: str) -> bh.Histogram:
    """
    Read a histogram produced with CORSIKA8's `SaveBoostHistogram()` function.

    Parameters
    ----------
    filename: str
        The filename of the .npy file containing the histogram.

    Returns
    -------
    hist: bh.Histogram
        An initialized bh.Histogram instance.

    Throws
    ------
    ValueError:
        If the histogram type is not supported.
    """

    # load the filenames
    d = np.load(filename)

    # extract the axis and overflows information
    axistypes = d["axistypes"].view("c")
    overflow = d["overflow"]
    underflow = d["underflow"]

    # this is where we store the axes that we extract from the file.
    axes = []

    # we now loop over the axes
    for i, (at, has_overflow, has_underflow) in enumerate(
        zip(axistypes, overflow, underflow)
    ):

        # continuous axis
        if at == b"c":
            axes.append(
                bh.axis.Variable(
                    d[f"binedges_{i}"], overflow=has_overflow, underflow=has_underflow
                )
            )
        # discrete axis
        elif at == b"d":
            axes.append(bh.axis.IntCategory(d[f"bins_{i}"], growth=(not has_overflow)))

        # and unknown histogram type
        else:
            raise ValueError(f"'{at}' is not a valid C8 histogram axistype.")

    # create the histogram and fill it in
    data = d["data"]
    if data.dtype.names: # structured array -> mean and variance
        h = bh.Histogram(*axes, storage=bh.storage.Weight())
        h.view(flow=True)[:]['value'] = data['mean']
        h.view(flow=True)[:]['variance'] = data['variance']
    else:
        h = bh.Histogram(*axes)
        h.view(flow=True)[:] = data

    return h
