#!/bin/bash
die () {
    echo >&2 "$@"
    echo "Usage: "
    echo "  makehtml.sh input.json output.html [rotX,rotY,rotZ]"
    echo "Or: "
    echo "  makehtml.sh input.gldb output.html gLucifer_bin [gLucifer_viewer_command_string]"
    exit 1
}

[ "$#" -ge 2 ] || die "2 arguments required, $# provided"
input=$1

# Get actual script location, including dereferenced symbolic links.
SOURCE="${BASH_SOURCE[0]}"
DIR="$( dirname "$SOURCE" )"
while [ -h "$SOURCE" ]
do
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
  DIR="$( cd -P "$( dirname "$SOURCE"  )" && pwd )"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"


if [[ $input == *.gldb ]]
then
  [ "$#" -ge 3 ] || die "3 arguments required, $# provided"
  echo "Generating JSON data... echo -e \"$4\" | $3 -j \"$input\" > temp.json"
  echo -e "$4" | $3 -j "$input" > temp.json
  json="temp.json"
  rot="none"
else
  json=$input
  rot=${3-none}
fi

if [[ $rot != "none" ]]
then
  echo "Setting initial rotation $rot"
  sed -i -e "s/\"rotate\" : \[[0-9\.,-]*\]/\"rotate\" : [$rot]/" $json
fi

echo "Generating standalone html from template..."
sed -e "/_JSON_DATA_/r $json" -e "/_JSON_DATA_/d" $DIR/template.html > $2

#Delete temp files
if [[ $json != $1 ]]
then
  rm $json
fi

