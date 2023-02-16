#! /bin/sh

DIR=$(readlink -f $(dirname $0))

if ! conan profile show corsika8 >/dev/null 2>/dev/null; then
  conan profile new --detect corsika8
  conan profile update settings.compiler.cppstd=17 corsika8
  if [ "$(uname)" = "Linux" ]; then
    conan profile update settings.compiler.libcxx=libstdc++11 corsika8
  fi
fi
# force rebuild of cubicinterpolation by removing it first. (see discussion in MR509)
conan remove cubicinterpolation/* --force
conan install -pr corsika8 --build=missing ${DIR}
