Creates a detailed callgraph viewable in Firefox for any function based on the tau profiler output file.
1 a: Run tau compiled version of Underworld on an .xml file (see tauSetup.txt)

1 b: set TAU_THROTTLE=0 (optional) to profile and completely count all functions 
     however many times they are called or how quickly they execute:
     by default tau will stop profiling functions that are called more than 100,000 times and take less then 10 usecs to run.

     set TAU_CALLPATH_DEPTH=5.

1 c: Tau profiled code creates a file called profile.0.0.0 for a serial run (1 cpu)

2: The scripts require a local copy of the source code (does not need to be compiled) that matches the run that created profile.0.0.0

3: You may rename the profile.0.0.0 to XY.0.0.0 then the scripts will create XY_funcname.svg
   after running
   ./creategraph.sh XY funcname $UNDERWORLD_DIR

4: Then run
   firefox XY_funcname.svg
   to view the graph.
