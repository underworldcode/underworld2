#!/bin/sh

if ! python -c 'import underworld'; then
	echo "Can't import underworld python module"
	exit 2
fi

./run_tests.py ../examples/*.ipynb ../user_guide/*.ipynb ../test/*
