#!/usr/bin/gnuplot
set terminal pngcairo size 640,480 enhanced font 'Arial, 11'
#set terminal png size 640,480 enhanced font 'Arial, 11'
set output 'ibarrier-blocking.png'

set border linewidth 1.2
set key top left
set grid
set mytics 0
set style line 1 linecolor rgb 'red' linetype 1 linewidth 2
set ylabel 'Time (us)'
set xlabel 'Number of processes'
set format y "%.1f"
set format x "%.0f"
set xtics 2
set mxtics 1
plot "ibarrier.dat" using 1:8 notitle with lines ls 1
     

