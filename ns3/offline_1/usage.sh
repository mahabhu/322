#!/bin/bash

echo "Usage:"
echo "./simulate_static.sh node:1  flow:2  pps:3    cf:4        xoption:5   yoption:6   upperbound:7    stepsize:8"
echo "./simulate_mobile.sh node:1  flow:2  pps:3    speed:4     xoption:5   yoption:6   upperbound:7    stepsize:8"
echo ""
echo "xoption:"
echo "1 : nWifi = 10; // 20, 40, 60, 80, 100"
echo "2 : flowNo = 10; // 10, 20, 30, 40, 50  -----> x 2"
echo "3 : packetPerSecond = 128; // 100, 200, 300, 400, 500"
echo "4 : speed = 10; // 5, 10, 15, 20 -----> m/s"
echo "4 : coverageFactor = 2; // 1, 2, 3, 4, 5 -----> *Tx Range"
echo ""
echo "yoption:"
echo "1 : Packet Delivery Ratio"
echo "2 : Throughput"
