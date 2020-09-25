"""
 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""
import corsika


def test_corsika_version() -> None:
    """
    Check the current CORSIKA version.
    """
    assert corsika.__version__ == "8.0.0-alpha"
