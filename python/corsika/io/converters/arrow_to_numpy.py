"""
 Converter for the pyarrow data types to numpy ones

 (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu

 This software is distributed under the terms of the 3-clause BSD license.
 See file LICENSE for a full version of the license.
"""

import numpy as np
import pyarrow


def convert_to_numpy(pyarrow_table: pyarrow.lib.Table) -> np.ndarray:
    """
    Converts a pyarrow Table to a numpy structured array

    Parameters
    ----------
    pyarrow_table: pyarrow.lib.Table
        PyArrow table of any dimension to be sliced

    Returns
    -------
    np.ndarray:
        converted table with the same column labels and data types

    """

    # Type conversions for pyarrow data types to numpy ones
    # https://arrow.apache.org/docs/python/data.html
    # https://numpy.org/doc/stable/reference/arrays.dtypes.html#arrays-dtypes-constructing
    type_conversions = {
        pyarrow.int8(): "int8",
        pyarrow.int16(): "int16",
        pyarrow.int32(): "int32",
        pyarrow.int64(): "int64",
        pyarrow.uint8(): "uint8",
        pyarrow.uint16(): "uint16",
        pyarrow.uint32(): "uint32",
        pyarrow.uint64(): "uint64",
        pyarrow.float16(): "float16",
        pyarrow.float32(): "float32",
        pyarrow.float64(): "float64",
    }

    # Perform type conversion of all fields
    column_types = [
        type_conversions[pyarrow_table[key].type] for key in pyarrow_table.column_names
    ]
    dtypes = [(x, y) for (x, y) in zip(pyarrow_table.column_names, column_types)]

    # Make an empty array and then fill the values
    np_table = np.zeros(pyarrow_table.num_rows, dtype=dtypes)
    for key in pyarrow_table.column_names:
        np_table[key] = pyarrow_table[key]

    return np_table
