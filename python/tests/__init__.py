"""
 Tests for `corsika`

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""

import os
import os.path as op

__all__ = ["build_directory"]


def find_build_directory() -> str:
    """
    Return the absolute path to the CORSIKA8 build directory.

    Returns
    -------
    str
        The absolute path to the build directory.

    Raises
    ------
    RuntimeError
        If the build directory cannot be found or it is empty.
    """

    # check if we are running on Gitlab
    gitlab_build_dir = os.getenv("CI_BUILDS_DIR")

    # if we are running on Gitlab
    if gitlab_build_dir is not None:
        build_dir = op.abspath(gitlab_build_dir)
    else:  # otherwise, we are running locally
        build_dir = op.abspath(
            op.join(
                op.dirname(__file__),
                op.pardir,
                op.pardir,
                "build",
            )
        )

    # check that the build directory contains 'CMakeCache.txt'
    if not op.exists(op.join(build_dir, "CMakeCache.txt")):
        raise RuntimeError("Python tests cannot find C8 build directory.")

    return build_dir


# find the build_directory once
build_directory = find_build_directory()
