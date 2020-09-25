##
# ##
# corsika
#
# @file

# find python3
PYTHON=`which python3`

# our testing targets
.PHONY: tests flake black isort all

all: mypy isort black flake tests

tests:
	${PYTHON} -m pytest --cov=corsika tests

flake:
	${PYTHON} -m flake8 corsika

black:
	${PYTHON} -m black -t py37 corsika tests

isort:
	${PYTHON} -m isort --atomic corsika tests

mypy:
	${PYTHON} -m mypy corsika

# end
