#! /bin/sh

DIR=$(readlink -f $(dirname $0))

if [ $# -eq 0 ]; then
  # no arguments passed, target is current working dir
  target_dir="$PWD"
elif [ $# -eq 1 ]; then
  # target is provided directory
  target_dir="$1"
else
  echo "usage: conan-install.sh [directory]" >&2
  exit 1
fi

echo "using `conan --version`"

if ! conan profile show corsika8 >/dev/null 2>/dev/null; then
  conan profile new --detect corsika8
  conan profile update settings.compiler.cppstd=17 corsika8
  if [ "$(uname)" = "Linux" ]; then
    conan profile update settings.compiler.libcxx=libstdc++11 corsika8
  fi
fi

mkdir -p "$target_dir" || exit 2
cd "$target_dir" || exit 3
conan remove -f arrow
conan install -pr corsika8 --build=missing "${DIR}"
