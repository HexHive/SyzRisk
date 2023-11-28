#!/bin/bash

for i in 2019-07-01 2020-07-01 2021-07-01 2022-07-01; do
  python plt_dist.py -m churn $i &
done

wait
