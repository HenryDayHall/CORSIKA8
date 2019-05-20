#!/bin/bash

command="clang-format -style=file `find . -iregex '^.*\.\(cc\|h\)$' -not -path './ThirdParty/*'`"

if [ "$1" == "check" ];
then
    ! ${command}  -output-replacements-xml | grep -c "<replacement " 
else
    ${command} -i
fi
