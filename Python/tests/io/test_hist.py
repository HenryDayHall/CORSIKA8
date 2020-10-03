"""
 Tests for `corsika.io.hist`

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import os
import os.path as op
import subprocess

import corsika

from .. import build_directory

# the directory containing 'testSaveBoostHistogram'
bindir = op.join(build_directory, "Framework", "Utilities")


def generate_hist() -> str:
    """
    Generate a test with `testSaveBoostHistogram`.

    Returns
    -------
    str
        The path to the generated histogram.
    bool
        If True, this file was regenerated.
    """

    # we construct the name of the bin
    bin = op.join(bindir, "testSaveBoostHistogram")

    # check if a histogram already exists
    if op.exists(op.join(bin, "hist.npz")):
        return op.join(bin, "hist.npz"), False
    else:
        # run the program - this generates "hist.npz" in the CWD
        subprocess.call(bin)

        return op.join(os.getcwd(), "hist.npz"), True


def test_corsika_io() -> None:
    """
    Test I can corsika.io without a further import.
    """
    corsika.io.read_hist


def test_corsika_read_hist() -> None:
    """
    Check that I can read in the test histograms with `read_hist`.
    """

    # generate a test histogram
    filename, delete = generate_hist()

    # and try and read in a histogram
    h = corsika.io.read_hist(filename)

    # and delete the generated histogram
    if delete:
        os.remove(filename)

    # and check that it isn't empty
    assert not h.empty()
