#!/usr/bin/env bash

opt -S -passes='function(place-safepoints),rewrite-statepoints-for-gc' test.ll -o test-opt.ll
llc --filetype=obj -O3 test-opt.ll -o test-opt.o


