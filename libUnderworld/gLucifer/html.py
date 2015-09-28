#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import glob
import re
from os.path import expanduser
import getopt
import time
import sqlite3
import json

from string import Template

step_t = Template('''
<div class=$class>
  <a href="#" id="$step" class="toggle" onclick="toggle(this); return false;"><div class="fixed">[ + ] Timestep $step</div></a>
  <div class="fixed">Time: $time</div>
  <div class="fixed">CPU Time: $cputime</div>
  <div id="step_$step" class="hidden">
    $images
    $stepfiles
    <div class="endfloat"></div>
  </div>
</div>

''')

image_t = Template('''
    <div class="windows">
      <a href="#" onclick="urlOpen('$window.$pstep.png');"><img class="thumb" onerror="this.style.display='none';" src="$window.$pstep.png"></a><br>
      <a href="#" onclick="urlOpen('$window.$pstep.jpg');"><img class="thumb" onerror="this.style.display='none';" src="$window.$pstep.jpg"></a><br>
      <!--a href="file://$uwpath/html/index.html?$outdir/$window.$pstep.jsonp">gLucifer Web Viewer</a-->
      <a href="#" onclick="urlOpen('html/index.html?../$window.$pstep.jsonp');" class="right">gLucifer Web Viewer</a><br>
      <!--a href="#" onclick="urlOpen('html/launcher.php?../$db&args=$step);" class="right;">Launch gLucifer Instance</a-->
    </div>
''')

file_t = Template('''
&nbsp;&nbsp;<a class="file" href="$filename">$filename</a><br>
''')

def usage():                         
    print "Usage..."
    print "-d,--database  filename (gldb file)"
    print "-o,--output    path (output files location)"
    print "-u,--uwpath    path (underworld location)"


def main(argv):
    #defaults
    home = expanduser("~")
    database = "gLucifer.gldb"
    outpath = home + "/stgUnderworld/output/"
    uwpath = home + "/stgUnderworld/build/bin/"

    try:
        opts, args = getopt.getopt(argv, "hd:o:u:j:", ["help", "database=", "output=", "uwpath=", "jobid="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ("-j", "--jobid"):
            jobid = arg
        elif opt in ("-d", "--database"):
            database = arg
        elif opt in ("-o", "--output"):
            outpath = arg
        elif opt in ("-u", "--uwpath"):
            uwpath = arg

    con = None

    #Read the main template, stored as separate file for ease of editing
    rt_path = os.path.join(os.path.normpath(uwpath), "results_template.html")
    with open(rt_path, "r") as tfile:
        page_t = Template(tfile.read())

    #Save db path
    dbpath = os.path.abspath(database)

    #Go to the output dir
    os.chdir(os.path.abspath(outpath))

    with open("input.xml", "r") as xmlfile:
        xml = xmlfile.read()

    data = {
            'db' : dbpath,
            'id' : jobid,
            'title' : 'Underworld Analysis',
            'date' : time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), 
            'uwpath' : uwpath,
            'outdir' : outpath,
            'xml' : xml
            }

    output = None
    with open("FrequentOutput.dat") as fofile:
        lines = fofile.read().split("\n")
        keys = lines[0].split()[1:]
        output = []
        for l in lines[1:]:
            output.append(dict(zip(keys, l.split())))

    try:
        con = sqlite3.connect(dbpath)
        
        cur = con.cursor()    
        cur.execute("SELECT id,time FROM timestep;")
        
        timesteps = cur.fetchall()

        cur.execute("SELECT name FROM window;")
        windows = cur.fetchall()

        #List of timesteps
        steps = ""
        count = 0
        for row in timesteps:
            ts5 = str(row[0]).zfill(5)
            images = ""
            for win in windows:
                img = data.copy()
                vals = {'step' : row[0], 
                        'pstep' : ts5,
                        'window' : win[0].encode('ascii')}
                img.update(vals)
                images = images + image_t.substitute(img)

            #List of per step files 
            files = ""
            for file in glob.glob("*." + ts5 + "*.h5"):
              files = files + file_t.substitute({"filename" : file})
            for file in glob.glob("*." + ts5 + "*.xmf"):
              files = files + file_t.substitute({"filename" : file})
            for file in glob.glob("*." + ts5 + "*.gldb"):
              files = files + file_t.substitute({"filename" : file})

            #Copy base data and update with additional values
            step = data.copy()
            vals = {'step' : row[0], 
                    'time' : row[1],
                    'cputime' : output[row[0]]['CPU_Time'],
                    'images' : images,
                    'class' : 'odd' if (count % 2 == 1) else 'even',
                    'stepfiles' : files
                    }

            step.update(vals)
            steps = steps + step_t.substitute(step)
            count += 1

        #Global list of files *.h5, *.gldb, *.xdmf, exclude timestep files
        files = ""
        for file in glob.glob("*.gldb"):
          if re.match('^((?![\d\d\d\d\d]).)*$', file) :
            files = files + file_t.substitute({"filename" : file})
        for file in glob.glob("*.h5"):
          if re.match('^((?![\d\d\d\d\d]).)*$', file) :
            files = files + file_t.substitute({"filename" : file})
        for file in glob.glob("*.dat"):
          if re.match('^((?![\d\d\d\d\d]).)*$', file) :
            files = files + file_t.substitute({"filename" : file})
        for file in glob.glob("*.xdmf"):
          if re.match('^((?![\d\d\d\d\d]).)*$', file) :
            files = files + file_t.substitute({"filename" : file})

        #Compose final data
        data.update({'steps' : steps, 'files' : files})
        print page_t.substitute(data)

        #json_data = json.dumps(rows)

        #print json_data
        

    except sqlite3.Error, e:
        
        print "Error %s:" % e.args[0]
        sys.exit(1)
        
    finally:
        
        if con:
            con.close()

if __name__ == "__main__":
    main(sys.argv[1:])
