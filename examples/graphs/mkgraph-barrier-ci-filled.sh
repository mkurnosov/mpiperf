#!/usr/bin/gnuplot
set terminal pngcairo size 640,480 enhanced font 'Arial, 11'
set output 'barrier-ci-filled.png'

set border linewidth 1.2
set key top left
set grid
set mytics 0
set style line 1 linecolor rgb 'red' linetype 1 linewidth 2
set style line 2 linecolor rgb 'blue' linetype 1 linewidth 1
set style line 3 linecolor rgb 'blue' linetype 1 linewidth 1
set ylabel 'Time (us)'
set xlabel 'Number of processes'
set format y "%.1f"
set format x "%.0f"
set xtics 8
set mxtics 4
set style fill solid 1.0 noborder
plot "barrier.dat" using 1:11:12 notitle with filledcurve ls 2,\
     "barrier.dat" using 1:11 title "Confidence interval" with lines ls 2, \
     "barrier.dat" using 1:12 notitle "CI UB" with lines ls 3,\
     "barrier.dat" using 1:5 title "Measured time" with lines ls 1
     

