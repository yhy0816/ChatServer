#!/bin/bash

set -x

mkdir `pwd`/build

cd `pwd`/build && rm -r * && cmake .. && make



