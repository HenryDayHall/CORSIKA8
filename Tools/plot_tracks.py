#!/usr/bin/env python3

import os
import sys, getopt
import re

# (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.

# with this script you can plot an animation of output of TrackWriter

tracks_dat="$1"
if [ -z "$tracks_dat" ]; then
  echo "usage: $0 <tracks.dat> [output.gif]" >&2
  exit 1
fi

directory=$(dirname "$tracks_dat")
hadrons_dat="$directory/hadrons.dat"
muons_dat="$directory/muons.dat"
electrons_dat="$directory/electrons.dat"
gammas_dat="$directory/gammas.dat"

#if [ "$muons_dat" -ot "$tracks_dat" -o "$hadrons_dat" -ot "$muons_dat" ]; then
  echo "splitting $tracks_dat into $muons_dat and $hadrons_dat."
  cat "$tracks_dat" | egrep '^\s+-*13\s' > "$muons_dat"
  cat "$tracks_dat" | egrep '^\s+-*11\s' > "$electrons_dat"
  cat "$tracks_dat" | egrep '^\s+-*22\s' > "$gammas_dat"
  cat "$tracks_dat" | egrep -v '^\s+-*13\s' | egrep -v '^\s+-*11\s' | egrep -v '^\s+-*22\s' > "$hadrons_dat"
#fi

output="$2"
if [ -z "$output" ]; then
  output="$tracks_dat.gif"
fi

echo "creating $output..."

cat <<EOF | gnuplot
set term gif animate size 900,900
set output "$output"

#set zrange [0:40e3]
#set xrange [-10:10]
#set yrange [-10:10]
set xlabel "x / m"
set ylabel "y / m"
set zlabel "z / m"
set title "CORSIKA 8 preliminary"

do for [t=0:359:1] {
# for separate file per angle:
#	set output sprintf("%03d_$output", t)

	set view 80, t
        splot "$gammas_dat" u 3:4:5:6:7:8 w vectors nohead lt rgb "orange" t "", "$electrons_dat" u 3:4:5:6:7:8 w vectors nohead lt rgb "blue" t "", "$muons_dat" u 3:4:5:6:7:8 w vectors nohead lt rgb "red" t "", "$hadrons_dat" u 3:4:5:6:7:8 w vectors nohead  lc rgb "black" t ""
}
EOF

exit $?
