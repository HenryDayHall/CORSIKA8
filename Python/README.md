# Python for CORSIKA8

![License](https://img.shields.io/badge/license-GPL3-blue)
![Python](https://img.shields.io/badge/python-3.6%20%7C%203.7%20%7C%203.8-blue)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

## Installation

To install the CORSIKA 8 Python library, you will need Python >= 3.6. The below
instructions are assuming that `python` refers to Python 3.\*. If `python` still
refers to Python 2.\*, please replace `python` with `python3` and `pip` with
`pip3`.

To install this package, change into the `corsika/Python` directory and run

    pip install --user -e .

You can verify that the installation was successful by trying to import `corsika`

    python -c 'import corsika'

# Running the Tests
    
If you wish to develop new features in `corsika`, you will also need to install
some additional dependencies so you can run our unit tests. These can be
installed with

    pip install --user -e .[test]

Once that is completed, you can run the unit tests directory from the `corsika` directory

    python -m pytest tests
