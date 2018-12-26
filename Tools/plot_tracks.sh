#!/bin/sh

# (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
#
# See file AUTHORS for a list of contributors.
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.

# with this script you can plot an animation of output of TrackWriter

track_dat=$1
if [ -z "$track_dat" ]; then
  echo "usage: $0 <track.dat> [output.gif]" >&2
  exit 1
fi

output=$2
if [ -z "$output" ]; then
  output="$track_dat.gif"
fi

cat <<EOF | gnuplot
set term gif animate size 600,600
set output "$output"

set xlabel "x / m"
set ylabel "y / m"
set zlabel "z / m"
set title "CORSIKA 8 preliminary"

do for [t=0:360:1] {
	set view 60, t
        splot "$track_dat" u 3:4:5:6:7:8 w vectors nohead t ""
}
EOF

exit $?
