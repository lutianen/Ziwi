#!/bin/bash

# Format
clang-format -i `find include/ -type f -name *.h`
clang-format -i `find src/ -type f -name *.cc`

# Count
# cloc --git `git branch --show-current`
