"""
 A Python interface to CORSIKA 8.

 (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the 3-clause BSD license.
 See file LICENSE for a full version of the license.
"""

import logging

from . import io
from .io.library import Library

logger = logging.getLogger("corsika")
fmt = "[%(levelname)s] - %(name)s - %(message)s"
myFormatter = logging.Formatter(fmt)
handler = logging.StreamHandler()
handler.setFormatter(myFormatter)
logger.addHandler(handler)

# all imported objects
__all__ = ["io", "Library"]

__version__: str = "8.0.0-alpha"
