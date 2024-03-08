from os import path
from setuptools import setup, find_packages

# the stereo version
__version__ = "8.0.0-alpha"

# get the absolute path of this project
here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, "README.md"), encoding="utf-8") as f:
    long_description = f.read()

# the standard setup info
setup(
    name="corsika",
    version=__version__,
    description="A Python package for working with CORSIKA 8.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://gitlab.iap.kit.edu/AirShowerPhysics/corsika",
    author="CORSIKA 8 Collaboration",
    author_email="corsika-devel@lists.kit.edu",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering :: Physics",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
    ],
    keywords=["cosmic ray", "physics", "air shower", "simulation"],
    packages=find_packages(),
    python_requires=">=3.6, <4",
    install_requires=["numpy", "pyyaml", "pyarrow", "pandas"],
    extras_require={
        "test": [
            "pytest",
            "black",
            "mypy",
            "isort",
            "coverage",
            "pytest-cov",
            "flake8",
            "types-PyYAML",
            "pandas-stubs",
        ],
        "examples": [
            "argparse",
            "matplotlib",
            "particle",
        ],
    },
    scripts=[],
    project_urls={"code": "https://gitlab.iap.kit.edu/AirShowerPhysics/corsika"},
    include_package_data=False,
)
