#!/bin/bash

set -x

mkdir `pwd`/build && rm -r `pwd`/build/*

cd `pwd`/build &&  cmake .. && make




