#!/usr/bin/env gnuplot
set autoscale
unset log
set logscale y
unset label
set xtic auto
set ytic auto
set title "Residuals"
set grid
set xlabel "Iterations"
plot "logs/p_rgh_0" using 1:2 title 'p' with lines, \
     "logs/Ux_0" using 1:2 title 'Ux' with lines, \
     "logs/Uy_0" using 1:2 title 'Uy' with lines, \
     "logs/Uz_0" using 1:2 title 'Uz' with lines, \
     "logs/k_0" using 1:2 title 'k' with lines, \
     "logs/k_0" using 1:2 title 'h' with lines

set size 1.0, 0.6
set terminal tkcanvas
pause -1  "Hit return to continue"
set terminal png medium font Helvetica 14
set output "foamLog.png"
replot


