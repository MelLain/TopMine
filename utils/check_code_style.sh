#!/bin/bash

echo "Running C++ code checks"
cat cpplint_files.txt | while read f
do
    cpplint --linelength=120 --filter=-legal/copyright,-runtime/int,-whitespace/braces,-build/c++11,-runtime/string $f
done
