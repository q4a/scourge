#!/bin/bash
export LIBGL_ALWAYS_INDIRECT=1
#valgrind --tool=memcheck --suppressions=./scourge.supp --gen-suppressions=yes ./src/scourge -fs
valgrind --tool=addrcheck --suppressions=./scourge.supp --gen-suppressions=yes --leak-check=yes -v ./src/scourge -fs
