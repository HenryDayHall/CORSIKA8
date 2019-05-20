clang-format -i -style=file `find . -iregex '^.*\.\(cc\|h\)$' -not -path './ThirdParty/*'`
