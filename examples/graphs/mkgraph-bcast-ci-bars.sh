#!/usr/bin/gnuplot
set terminal pngcairo size 640,480 enhanced font 'Arial, 11'
set output 'bcast-ci-bars.png'
#set terminal postscript eps enhanced color

set border linewidth 1.2
set key top left
set nogrid
set mytics 0
set style line 1 linecolor rgb 'red' linetype 1 linewidth 2
set style line 2 linecolor rgb 'blue' linetype -1 linewidth 1 pt -1
set ylabel 'Time (us)'
set xlabel 'Message size (bytes)'
set format y "%.1f"
set format x "%.0f"

set logscale x
set xtics 4
set mxtics 2
plot "bcast.dat" using 2:6 notitle with lines ls 1, \
     "bcast.dat" using 2:6:11 notitle with yerrorbars ls 2
     
