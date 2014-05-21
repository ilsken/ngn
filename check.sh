#!/bin/bash

HEADERS=""
NL=$'\n'

for header in "$@"
do
    HEADERS="$HEADERS #include <$header>$NL"
  done
echo "$HEADERS"

echo "$HEADER" | LC_MESSAGES=en_US gcc -P -E -xc - 2>&1
