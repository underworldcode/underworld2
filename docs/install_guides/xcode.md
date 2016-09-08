Xcode development environment setup
===================================

These instructions are valid for xcode 7.3. Unfortunately xcode move pretty quick, so some of the options/menus might exist in different locations, but the general procedure should still work.  

Setup to build
--------------
1.  Create a new xcode project.
2.  Select "Other"->"External Build System".
3.  Set your product name, organisation & identifier as you wish (ignore build tool for now)
4.  Decide where the xcode project should be saved. It's probably better to keep this seperate from your uw2 codebase.
5.  Go to "File"->"Add files to...".  Click "Options" and deselect "Copy items if needed". Find the top level of your uw2 codebase, and click "Add".
6.  Select the top level item of your project tree (blue icon) in the left panel, then select your "Target" in the next column, and Select the "Info" panel.
7.  For "Build Tool", enter "/bin/sh" (or whatever your path to bash or sh is).
8.  For "Arguments" you will need to set your path environment along with the required build executable ("scons.py"). For example:
    "-c  'PATH=/usr/bin:/bin:/usr/local/bin  /Users/jmansour/work/code/underworld2_development/libUnderworld/scons.py'"
9.  For "Directory", set your full path to libUnderworld.  For example:
    "/Users/jmansour/work/code/underworld2_development/libUnderworld/"
10. Uncheck "Pass build settings in environment".

Setup for autocomplete/cscope type functionality
------------------------------------------------
We wish to add a new target to our project. Adding this target will result in xcode parsing the underworld source code. 
1.  Select the top level item of your project tree (blue icon) in the left panel, then click "+" in the next column.  
2.  Select "OS X"->"Frameworks & Library"->"Library", click "Next".
3.  Give your target any name, and in the "Framework" field, select "None (Plain C/C++ Library). For "Type", anything should work. I have selected "Dynamic". 
4.  In your newly created Target, go to the "Build Phases" panel, select "Compile Sources", and click "+". Then in the dialogue, select all and click add. It should be around 5000 files. 

The indexing may take a while, and it may also be necessary to restart xcode for the code sense to become active. 
