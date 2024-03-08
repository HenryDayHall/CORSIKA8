##
# ##
# corsika
#
# @file

# find python3
PYTHON=`which python3`

# our testing targets
.PHONY: tests flake black isort all

all: black flake isort mypy tests

tests:
	${PYTHON} -m pytest --cov=corsika tests

flake:
	${PYTHON} -m flake8 corsika tests examples

black:
	${PYTHON} -m black -t py37 corsika tests examples

isort:
	${PYTHON} -m isort --atomic corsika tests examples

mypy:
	${PYTHON} -m mypy corsika

# end
