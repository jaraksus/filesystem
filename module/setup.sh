#!/bin/bash

make
make test
sudo mknod /dev/lkm_minifs c 238 0