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

flukalibOrig="$1"
target="$2"

tmpdir=`mktemp -d fluka_objectsXXXXXX`

echo "extracting objects from $1 into `realpath $tmpdir`..."
ar --output "$tmpdir" x "$flukalibOrig"
rm "$tmpdir/flrndm.o"

[ -f "libflukahp-norndm.a" ] && rm "libflukahp-norndm.a"

echo "creating libflukahp-norndm.a..."
ar -rcs "$target/libflukahp-norndm.a" "$tmpdir"/*.o

rm -r "$tmpdir"
