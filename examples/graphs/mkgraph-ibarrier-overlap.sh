#!/usr/bin/gnuplot
set terminal pngcairo size 640,480 enhanced font 'Arial, 11'
#set terminal png size 640,480 enhanced font 'Arial, 11'
set output 'ibarrier-overlap.png'

set border linewidth 1.2
set key top left
set grid
set style line 1 linecolor rgb 'red' linetype 1 linewidth 2
set ylabel 'Overlap'
set xlabel 'Computation time (usec)'
set format y "%1.2f"
plot "ibarrier.dat" using 4:12 notitle with lines ls 1
     

