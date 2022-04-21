#!/bin/bash

./tso &

./tsd -p 3111 -d 0 -t 0 &
./tsd -p 3112 -d 1 -t 0 &
./tsd -p 3113 -d 2 -t 0 &

./tsd -p 3114 -d 0 -t 1 &
./tsd -p 3115 -d 1 -t 1 &
./tsd -p 3116 -d 2 -t 1 &