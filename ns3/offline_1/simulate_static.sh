#!/bin/bash


xvalue="hello"
yvalue="hello"

cp scratch/1905069/1905069_static.cc scratch/1905069_static.cc
./ns3 run "scratch/1905069_static.cc" > scratch/1905069/graph_static.dat


rm scratch/1905069/graph_static.dat
touch scratch/1905069/graph_static.dat

# int nWifi = 10; // 20, 40, 60, 80, 100
# int flowNo = 10; // 10, 20, 30, 40, 50  -----> x 2
# double packetPerSecond = 128; // 100, 200, 300, 400, 500
# double coverageFactor = 2; // 1, 2, 3, 4, 5 -----> *Tx Range



if [ $5 = 1 ]
then
    xvalue="No. of Nodes"
    let node=$1
    while [ $node -le $7 ]
    do
        echo "Running simulation with $node nodes on each side"
        ./ns3 run "scratch/1905069_static.cc $node $2 $3 $4 $5 $6" >> scratch/1905069/graph_static.dat
        echo "Simulation with node = $node completed"
        let node=$node+$8
    done
elif [ $5 = 2 ]
then
    xvalue="No. of Flows"
    let flow=$2
    while [ $flow -le $7 ]
    do
        echo "Running simulation with $flow flows"
        ./ns3 run "scratch/1905069_static.cc $1 $flow $3 $4 $5 $6" >> scratch/1905069/graph_static.dat
        echo "Simulation with flow = $flow completed"
        let flow=$flow+$8
    done
elif [ $5 = 3 ]
then
    xvalue="Packets per Second"
    let pps=$3
    while [ $pps -le $7 ]
    do
        echo "Running simulation with $pps packets per second"
        ./ns3 run "scratch/1905069_static.cc $1 $2 $pps $4 $5 $6" >> scratch/1905069/graph_static.dat
        echo "Simulation with packet per second = $pps completed"
        let pps=$pps+$8
    done
elif [ $5 = 4 ]
then
    xvalue="Coverage Factor"
    let cf=$4
    while [ $cf -le $7 ]
    do
        echo "Running simulation with coverage factor $cf"
        ./ns3 run "scratch/1905069_static.cc $1 $2 $3 $cf $5 $6" >> scratch/1905069/graph_static.dat
        echo "Simulation with coverage factor = $cf"
        let cf=$cf+$8
    done
fi

if [ $6 = 1 ]
then
    yvalue="Packet Delivery Ratio"
elif [ $6 = 2 ]
then
    yvalue="Throughput"
fi



echo "All simulations completed"

gnuplot -persist <<-EOFMarker
    set terminal png size 640,480
    set output "scratch/1905069/graph_static.png"
    plot "scratch/1905069/graph_static.dat" using 1:2 title '$yvalue vs $xvalue' with linespoints
EOFMarker

echo "plotted"

