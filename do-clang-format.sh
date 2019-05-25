#!/bin/bash

command="clang-format -style=file `find . -iregex '^.*\.\(cc\|h\)$' -not -path './ThirdParty/*'`"

if [ "$1" == "check" ];
then
    `! ${command}  -output-replacements-xml | grep -qc "<replacement "` || \
        { echo "format-check FAILED!"; exit $ERRCODE; }
    echo "Congratulations: format-check succeeded"
elif [ "$1" == "apply" ];
then
    ${command} -i
else
    echo "please use: ./do-clang-format.sh [check] or [apply]"
fi
