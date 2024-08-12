#!/usr/bin/env bash

opt -S -passes='function(place-safepoints),module(rewrite-statepoints-for-gc)' add.ll -o add.opt.ll
llc --filetype=obj -O3 add.opt.ll -o add.o
llc --filetype=asm --x86-asm-syntax=intel -O3 add.opt.ll -o add.s

opt -S -passes='function(place-safepoints),module(rewrite-statepoints-for-gc)' main.ll -o main.opt.ll
llc --filetype=obj -O3 main.opt.ll -o main.o
llc --filetype=asm --x86-asm-syntax=intel -O3 main.opt.ll -o main.s


