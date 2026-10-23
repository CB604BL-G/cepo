#!/bin/sh
set -e

if [ ! -d ".build" ]; then
	rm -rf "./.build"
	cmake -S . -B ".build"
fi
cmake --build ".build"
./.build/test_dependency_of_cepo