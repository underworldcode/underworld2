import sys
import re
import lavavu

if len(sys.argv) < 2:
  print "Convert text xyz to LavaVu database"
  print "  Usage: "
  print "    xyz2gldb.py input.txt output.gldb"
  print "      Convert with varying bounding box"
  print "    xyz2gldb.py input.txt output.gldb 1" 
  print "      Convert with bounding box fixed to maximum over full simulation"
  print ""
  exit()

filePath = sys.argv[1] #cmds.fileDialog()
dbPath = sys.argv[2]
#Optional: fix bounding box to min/max over entire dataset
fixBB = 0
if len(sys.argv) > 3:
    fixBB = sys.argv[3]

#Create a colourmap with 15 random colours
colours = lavavu.RandomColourMap(count=15, seed=34)
colours2 = lavavu.RandomColourMap(count=15, seed=1)
#Print the colour list, this can be used as the starting point for a custom colour map
print colours.colours
#User defined colour map (R,G,B [0,1])
#colours = lavavu.ColourMap([[1,0,0], [1,1,0], [0,1,0], [0,1,1], [0,0,1], [1,0,1], [1,1,1]])
#Create vis object, points and lines
points = lavavu.Points('molecules', colours, size=10, pointtype=lavavu.Points.ShinySphere)
endpoints = lavavu.Points('molecule-ends', colours2, size=11, pointtype=lavavu.Points.ShinySphere)
#points = lavavu.Shapes('molecules', colours, size=1.0, shape=lavavu.Shapes.Sphere)
#endpoints = lavavu.Shapes('molecule-ends', colours2, size=1.1, shape=lavavu.Shapes.Sphere)
#points = lavavu.Points('molecules', None, size=10, pointtype=lavavu.Points.ShinySphere, props="colour=[1,0,0]")
links = lavavu.Lines('links', colours, width=1, link=True, flat=True)
#Create vis database for output
db = lavavu.Database(dbPath)
#Write the colourmaps to the database
colours.write(db)
colours2.write(db)

#For storing bounding box over full simulation
bbmin = [float("inf"),float("inf"),float("inf")]
bbmax = [float("-inf"),float("-inf"),float("-inf")]

#Function to write a stored chain of molecules to database
def writeChain():
  global db, points, endpoints, links, bbmin, bbmax
  #Set value min/max to number of chains
  points.vmin = links.vmin = endpoints.vmin = 0
  points.vmax = links.vmax = endpoints.vmax = 15
  #Save min/max bounding box dims
  if fixBB:
    for i in range(3):
      if bbmin[i] > points.bmin[i]: bbmin[i] = points.bmin[i]
      if bbmax[i] < points.bmax[i]: bbmax[i] = points.bmax[i]

  #Write end-points using first and last vertices of chain
  endpoints.addVertex(points.vertices[0], points.vertices[1], points.vertices[2])
  endpoints.addValue(points.values[0])
  last = len(points.vertices)-1
  endpoints.addVertex(points.vertices[last-2], points.vertices[last-1], points.vertices[last])
  endpoints.addValue(points.values[len(points.values)-1])
  #Trim end points from main chain
  points.vertices = points.vertices[3:last-3]

  points.write(db)
  links.write(db)
  endpoints.write(db)

#Loop over lines in input file
with open(filePath, 'r') as file:
  lastnum = None

  for line in file:
    if re.match(r'\AAtoms', line):
      #Found a new timestep
      if lastnum: 
        #Write saved data from previous chain
        writeChain();
      #Create a new timestep entry in database
      db.timestep()
      lastnum = None
      pass
    elif len(line) <= 2:
      #Skip blank lines
      pass
    elif (line.count(' ') >= 3):
      #Read particle position
      data = line.split()
      x = float(data[1])
      y = float(data[2])
      z = float(data[3])
      num = int(data[0])

      if lastnum and lastnum != num:
        #Save and start a new chain when id number changes
        writeChain()

      #Write vertex position, Points...
      points.addVertex(x, y, z)
      #... and Linking lines
      links.addVertex(x, y, z)

      #R,G,B colour if provided
      if len(data) >= 6:
        rgba = [int(data[4]), int(data[5]), int(data[6]), 255]
        points.addColour(rgba)
        links.addColour(rgba)

      #Write chain id as value for colouring
      #if lastnum != num:
      #if lastnum: print "old %d new %d" % (lastnum, num)
      links.addValue(num)
      points.addValue(num)

      lastnum = num

  #Write remaining saved data
  if lastnum: writeChain()

  #Write window (sets global bounding box dims)
  if fixBB:
    db.addWindow("", bbmin, bbmax, "")

  #Close and write database to disk
  db.close()


