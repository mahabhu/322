#!/bin/bash

# cp scratch/transport/$1.cc scratch/$1.cc
# ./ns3 run "scratch/$1.cc" #> scratch/transport/$2.dat 2>&1

algo0="ns3::TcpNewReno"
algo1="ns3::TcpAdaptiveReno"

# "ns3::TcpWestwoodPlus"
# "ns3::TcpNewReno"
# "ns3::TcpHighSpeed"
# "ns3::TcpAdaptiveReno"

rm -rf scratch/transport/throughput_vs_capacity.dat
touch scratch/transport/throughput_vs_capacity.dat
i=1
while [ $i -le 300 ]
do
    ./ns3 run "scratch/1905069.cc $i 6 scratch/transport/throughput_vs_capacity.dat $algo0 $algo1"
    gnuplot -persist <<-EOFMarker
        set terminal png size 640,480
        set output "scratch/cwnd/cwnd_vs_t-$i-6.png"
        plot "scratch/cwnd/cwnd_vs_t-$i-6-$algo0.dat" using 1:2 ls 1 title '$algo0' with linespoints ,\
             "scratch/cwnd/cwnd_vs_t-$i-6-$algo1.dat" using 1:2 ls 2 title '$algo1' with linespoints
EOFMarker
    if [ $i -lt 10 ]; then
        let i=$i+1
    elif [ $i -lt 50 ]; then
        let i=$i+10
    else
        let i=$i+50
    fi
done

rm -rf scratch/transport/throughput_vs_lossrate.dat
touch scratch/transport/throughput_vs_lossrate.dat
i=6
while [ $i -ge 2 ]
do
    ./ns3 run "scratch/1905069.cc 50 $i scratch/transport/throughput_vs_lossrate.dat $algo0 $algo1"
    gnuplot -persist <<-EOFMarker
        set terminal png size 640,480
        set output "scratch/cwnd/cwnd_vs_t-50-$i.png"
        plot "scratch/cwnd/cwnd_vs_t-50-$i-$algo0.dat" using 1:2 ls 1 title '$algo0' with linespoints ,\
             "scratch/cwnd/cwnd_vs_t-50-$i-$algo1.dat" using 1:2 ls 2 title '$algo1' with linespoints
EOFMarker
    let i=$i-1
done

gnuplot -persist <<-EOFMarker
    set terminal png size 640,480
    set output "scratch/transport/throughput_vs_capacity-$algo0-$algo1.png"
    plot "scratch/transport/throughput_vs_capacity.dat" using 1:3 ls 1 title 'throughput vs capacity for $algo0' with linespoints ,\
         "scratch/transport/throughput_vs_capacity.dat" using 1:4 ls 2 title 'throughput vs capacity for $algo1' with linespoints 
    set output "scratch/transport/throughput_vs_lossrate-$algo0-$algo1.png"
    plot "scratch/transport/throughput_vs_lossrate.dat" using 2:3 ls 1 title 'throughput vs lossrate for $algo0' with linespoints ,\
         "scratch/transport/throughput_vs_lossrate.dat" using 2:4 ls 2 title 'throughput vs lossrate for $algo1' with linespoints 
    set output "scratch/transport/fairness_vs_capacity-$algo0-$algo1.png"
    plot "scratch/transport/throughput_vs_capacity.dat" using 1:5 title 'fairness vs capacity' with linespoints
    set output "scratch/transport/fairness_vs_lossrate-$algo0-$algo1.png"
    plot "scratch/transport/throughput_vs_lossrate.dat" using 2:5 title 'fairness vs lossrate' with linespoints 
EOFMarker

