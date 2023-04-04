#!/bin/sh

# (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
#
# See file AUTHORS for a list of contributors.
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.

# This script strips off flrndm() from the libflukahp.a so that we can provide our own
# implementation.

flukalibOrig=`realpath $1`
target="$2"

if [ ! -r "$flukalibOrig" ]; then
    echo "\"$flukalibOrig\" not readable" 1>&2
    exit 1
fi

tmpdir=`mktemp -d fluka_objectsXXXXXX`
workdir=`pwd`

echo "extracting objects from $flukalibOrig into `realpath $tmpdir`..."
cd "$tmpdir"
ar x "$flukalibOrig"
cd "$workdir"
rm "$tmpdir/flrndm.o"

[ -f "libflukahp-norndm.a" ] && rm "libflukahp-norndm.a"

echo "creating libflukahp-norndm.a..."
ar -rcs "$target/libflukahp-norndm.a" "$tmpdir"/*.o

rm -r "$tmpdir"
