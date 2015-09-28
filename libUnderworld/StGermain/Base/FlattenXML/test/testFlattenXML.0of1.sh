#!/bin/sh

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

runAndHandleSystemTestStdLocations "FlattenXML data/testInput.xml -output_file=./output/output.xml" "$0" "$@"
