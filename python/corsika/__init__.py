"""
 A Python interface to CORSIKA 8.

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the GNU General Public
 Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 the license.
"""

from . import io
from .io.library import Library

# all imported objects
__all__ = ["io", "Library"]

__version__: str = "8.0.0-alpha"
