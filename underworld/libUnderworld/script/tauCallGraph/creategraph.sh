#example 
# ./creategraph.sh FMM3Dprofile WeightsCalculator_CalculateAll /home/mvelic/uworld/stgUnderworldWarMad
# where FMM3Dprofile is prefix name of FMM3Dprofile.0.0.0
# the default profile file name is profile.0.0.0 for processor 0
OUT="$1"."out"
OUTALL="$1"."outall"
FUNC=$2
UWD=$3
PREF="$1"_"$FUNC"
CDOT="$FUNC"."dot"
DOT="$PREF"."dot"
DOT2="$PREF"2."dot"
SVG="$PREF"."svg"
SVG2="$PREF"2."svg"
#echo "$FUNC $DOT $DOT2 $SVG $SVG2"
pprof -l -f $1  > $OUT
pprof -a -p -f $1 > $OUTALL
./getmain.pl $OUT $FUNC
mv $CDOT $DOT
#echo "$OUTALL $DOT $DOTP $UWD"
./postproc.pl $OUTALL $DOT $UWD
dot -Tsvg $DOT2 -o $SVG
./addscript.pl < $SVG > $SVG2
mv $SVG2 $SVG
mv $DOT2 $DOT

