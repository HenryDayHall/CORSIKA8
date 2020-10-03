"""
 The 'io' module provides for reading CORSIKA8 output files.

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""

from .hist import read_hist

# all exported objects
__all__ = ["read_hist"]
