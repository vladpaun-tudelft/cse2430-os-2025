#!/bin/bash
set -e

objcopy --dump-section .sha=temp $1
objcopy --remove-section .sha $1
openssl dgst -verify $2 -signature temp $1
./$1