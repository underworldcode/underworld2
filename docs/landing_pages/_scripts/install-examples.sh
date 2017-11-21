#!/bin/sh

set -e
cd $(dirname "$0")/..

cp -r ExampleContent Content
cp    ExampleContent/_config.yml _config.yml
