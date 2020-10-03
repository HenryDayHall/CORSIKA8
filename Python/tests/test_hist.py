"""
 Tests for `corsika.io.hist`

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import pytest

import corsika


def test_corsika_io() -> None:
    """
    Test I can corsika.io without a further import.
    """
    corsika.io.read_hist


def test_corsika_read_hist() -> None:
    """
    Check that I can read in the test histograms with `read_hist`.
    """

    # try and read in a continuous histogram

    # try and read in a discrete histogram


def test_corsika_read_hist_fail() -> None:
    """
    Check that an exception is thrown when reading
    an incorrectly formatted histogram.
    """
