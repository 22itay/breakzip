#!/bin/bash

mkdir -p stage1
build.out/src/mitm_stage1/mitm_stage1 -target ../nullified.zip -output stage1/nullified -shard_size 1000

NUM_CORES=2
SHARD_BEGIN=0
SHARD_END=21

seq ${SHARD_BEGIN} ${SHARD_END} | xargs -I % mkdir -p stage2/% ; 
seq ${SHARD_BEGIN} ${SHARD_END} | xargs -t -I % -P ${NUM_CORES} sh -c "build.out/src/mitm_stage2/mitm_stage2 -target ../nullified.zip -input_shard stage1/nullified.% -output stage2/%/nullified >stage2/%/stage2.log 2>&1;"

echo "Processed shards ${SHARD_BEGIN} through ${SHARD_END}"

