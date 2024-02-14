"""
 Tests for `corsika.io.outputs.energy_loss`

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""

import os
import os.path as op
import subprocess

import pytest

from corsika.io import Library

from .. import build_directory

bindir = op.join(build_directory, "bin")


def generate_data() -> str:
    """
    Generate a test with `testOutput`.

    Returns
    -------
    str
        The path to the generated data.
    """

    # Expected output directory that is made from the testOutput binary
    output_dir = op.join(os.getcwd(), "out_test/check")

    if not op.exists(output_dir):  # only make if not already run
        binary = op.join(bindir, "testOutput")

        # ensure that the binary exists (not trivial on the CI)
        if not op.exists(binary):
            msg = f"Could not find testOutput binary at {binary}\n"
            msg += f"Binary dir contains {os.listdir(bindir)}"
            raise RuntimeError(msg)

        subprocess.call([binary, "OutputManager"])

        # Check if it still doesn't exist
        if not op.exists(output_dir):
            msg = "After running binary, could not find expected"
            msg += f" output dir {output_dir}\n"
            msg += "The binary did not execute successfully or the"
            msg += " OutputManager tests have changed"
            raise RuntimeError(msg)

    return output_dir


def test_basic_Library() -> None:
    dir_to_test = generate_data()
    lib = Library(dir_to_test)

    assert 0 == len(lib.names)
    assert len(lib.summary)
    assert len(lib.config)

    # Check what happens for an unknown output subdir
    assert lib.get("Does not exits") is None


def test_bad_Library() -> None:
    with pytest.raises(ValueError):
        Library("This does not exist")
