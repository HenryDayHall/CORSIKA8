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
muon_dat=$2

if [ -z "$track_dat" ] || [ -z "$muon_dat" ]; then
  echo "usage: $0 <hadron.dat> <muon.dat> [output.gif]" >&2
  exit 1
fi

output=$3
if [ -z "$output" ]; then
  output="$track_dat.gif"
fi

cat <<EOF | gnuplot
set term png size 900,900
#set output "$output"

#set zrange [0:40e3]
#set xrange [-10:10]
#set yrange [-10:10]
set xlabel "x / m"
set ylabel "y / m"
set zlabel "z / m"
set title "CORSIKA 8 preliminary"

do for [t=0:359:1] {
	set output sprintf("%03d_$output", t)
	set view 90, t
        splot "$muon_dat" u 3:4:5:6:7:8 w vectors nohead lt rgb "red" t "", "$track_dat" u 3:4:5:6:7:8 w vectors nohead  lc rgb "black" t ""
}
EOF

exit $?
