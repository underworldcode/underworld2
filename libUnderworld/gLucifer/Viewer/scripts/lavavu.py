import sys
import re
import sqlite3
import struct
import random
from abc import ABCMeta, abstractmethod

class Type:
  Label = 0
  Point = 1
  Grid = 2
  Triangle = 3
  Vector = 4
  Tracer = 5
  Line = 6
  Shape = 7
  Volume = 8

class Data:
  Vertex = 0
  Normal = 1
  Vector = 2
  ColourValue = 3
  OpacityValue = 4
  RedValue = 5
  GreenValue = 6
  BlueValue = 7
  Index = 8
  XWidth = 9
  YHeight = 10
  ZLength = 11
  RGBA = 12
  TexCoord = 13
  Size = 14

class Database(object):
  def __init__(self, dbPath):
    self.step = 0
    self.objects = []
    self.path = dbPath

    #Create db
    self.db = sqlite3.connect(dbPath)

    self.db.execute('drop table if exists geometry')
    self.db.execute('drop table if exists timestep')
    self.db.execute('drop table if exists object')
    self.db.execute('drop table if exists object_colourmap')
    self.db.execute('drop table if exists colourmap')
    self.db.execute('drop table if exists colourvalue')
    self.db.execute('drop table if exists window')

    self.db.execute(("create table geometry "
                     "(id INTEGER PRIMARY KEY ASC, object_id INTEGER, timestep INTEGER, "
                     "rank INTEGER, idx INTEGER, type INTEGER, data_type INTEGER, size INTEGER, "
                     "count INTEGER, width INTEGER, minimum REAL, maximum REAL, dim_factor REAL, units VARCHAR(32), "
                     "minX REAL, minY REAL, minZ REAL, maxX REAL, maxY REAL, maxZ REAL, "
                     "labels VARCHAR(2048), properties VARCHAR(2048), data BLOB, "
                     "FOREIGN KEY (object_id) REFERENCES object (id) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "FOREIGN KEY (timestep) REFERENCES timestep (id) ON DELETE CASCADE ON UPDATE CASCADE)"))

    self.db.execute(("create table timestep (id INTEGER PRIMARY KEY ASC, time REAL, "
                     "dim_factor REAL, units VARCHAR(32), properties VARCHAR(2048))"))

    self.db.execute(("create table object (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), "
                    "colourmap_id INTEGER, colour INTEGER, opacity REAL, properties VARCHAR(2048), "
                    "FOREIGN KEY (colourmap_id) REFERENCES colourmap (id) ON DELETE CASCADE ON UPDATE CASCADE)"))

    self.db.execute(("create table object_colourmap (id integer primary key asc, "
                     "object_id integer, colourmap_id integer, data_type integer, "
                     "foreign key (object_id) references object (id) on delete cascade on update cascade, "
                     "foreign key (colourmap_id) references colourmap (id) on delete cascade on update cascade)"))

    self.db.execute(("create table colourvalue (id INTEGER PRIMARY KEY ASC, "
                     "colourmap_id INTEGER, colour INTEGER, value REAL, "
                     "FOREIGN KEY (colourmap_id) REFERENCES colourmap (id) ON DELETE CASCADE ON UPDATE CASCADE)")) 

    self.db.execute(("create table colourmap (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), minimum REAL, maximum REAL, "
                     "logscale INTEGER, discrete INTEGER, centreValue REAL, properties VARCHAR(2048))"))

    self.db.execute(("create table window (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), "
                     "width INTEGER, height INTEGER, colour INTEGER, "
                     "minX REAL, minY REAL, minZ REAL, maxX REAL, maxY REAL, maxZ REAL, properties VARCHAR(2048))"))

    self.db.commit()

    self.cursor = self.db.cursor()

  def close(self):
    self.db.commit()
    self.db.close()

  def timestep(self):
    self.step += 1
    print "Time step: " + str(self.step)
    self.db.execute("insert into timestep (id, time, dim_factor, units) values (%d, %f, 0, '')" % (self.step, float(self.step)))

  def addWindow(self, name, bmin, bmax, props):
    self.db.execute(("insert into window (name, minX, minY, minZ, maxX, maxY, maxZ, properties) "
                     "values ('%s', %g, %g, %g, %g, %g, %g, '%s')" %
                     (name, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], props)))

  def addObject(self, name, props):
    self.db.execute(("insert into object (id, name, colourmap_id, colour, opacity, properties) "
                     "values (1, 'atoms', 1, -1, 1.0, 'pointsize=10\npointtype=3')"))

  def insertGeometry(self, obj, dtype, size, count, width, minimum, maximum, bmin, bmax, data):
    if bmin and bmax:
      query = ("insert into geometry (object_id, timestep, type, data_type, "
               "size, count, width, minimum, maximum, "
               "minX, minY, minZ, maxX, maxY, maxZ, labels, data) "
               "values (%d, %d, %d, %d, %d, %d, %d, %g, %g, %g, %g, %g, %g, %g, %g, '%s', ?)"
               % (obj.id, self.step, obj.otype, dtype, size, count, width, minimum, maximum, 
                  bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], ""))
    else:
      query = ("insert into geometry (object_id, timestep, type, data_type, "
               "size, count, width, minimum, maximum, labels, data) "
               "values (%d, %d, %d, %d, %d, %d, %d, %g, %g, '%s', ?)"
               % (obj.id, self.step, obj.otype, dtype, size, count, width, minimum, maximum, ""))
    #print query
    self.db.execute(query, [buffer(data)]);

class ColourMap(object):
  def __init__(self, colours):
    self.colours = colours

  def write(self, db):
    query = ("insert into colourmap (name, minimum, maximum, logscale, discrete, centreValue) "
             "values ('random', 1, %d, 0, 0, 0)" % len(self.colours))
    db.cursor.execute(query)
    self.id = db.cursor.lastrowid

    for i in range(len(self.colours)):
      db.cursor.execute(("insert into colourvalue (colourmap_id, colour, value) values (%d, %d, %g)"
                         % (self.id, self.colourToInt(self.colours[i]), float(i+1))))

  @staticmethod
  def colourToInt(c):
    #ARGB int
    if len(c) < 4: c.append(1.0)
    return int(c[3]*255) << 24 | int(c[2]*255) << 16 | int(c[1]*255) << 8 | int(c[0]*255)

class RandomColourMap(ColourMap):
  def __init__(self, count, seed):
    random.seed(seed) #fixed seed for repeatable random colours
    colours = []
    for i in range(1, count):
      colours.append(self.randomColour())
    super(RandomColourMap, self).__init__(colours)

  @staticmethod
  def randomColour():
    return [random.randint(0,255)/255.0, random.randint(0,255)/255.0, random.randint(0,255)/255.0, 1.0]

class DrawingObject(object):
  #Abstract class for drawing objects
  __metaclass__ = ABCMeta

  def __init__(self, name, cmap, props):
    self.name = name
    self.cmap = cmap
    self.props = props
    self.clear()
    self.id = None

  def addVertex(self, x, y, z):
    self.vertices.append(x)
    self.vertices.append(y)
    self.vertices.append(z)
    #Update bounding box
    if x > self.bmax[0]: self.bmax[0] = x;
    if y > self.bmax[1]: self.bmax[1] = y;
    if z > self.bmax[2]: self.bmax[2] = z;

    if x < self.bmin[0]: self.bmin[0] = x;
    if y < self.bmin[1]: self.bmin[1] = y;
    if z < self.bmin[2]: self.bmin[2] = z;

  def addValue(self, v):
    val = float(v)
    self.values.append(val)
    if val < self.vmin: self.vmin = val;
    if val > self.vmax: self.vmax = val;
    
  def addColour(self, rgba):
    #Colours stored as 4 byte rgba array
    colour = rgba[0] | rgba[1] << 8 | rgba[2] << 16 | rgba[3] << 24
    self.colours.append(colour)

  def addWidth(self, v):
    self.widths.append(v)

  def addHeight(self, v):
    self.heights.append(v)

  def addLength(self, v):
    self.lengths.append(v)

  def write(self, db):
    if not self.id:
      cmapid = 0
      if self.cmap: cmapid = self.cmap.id
      query = ("insert into object (name, colourmap_id, colour, opacity, properties) "
               "values ('%s', %d, -1, 1.0, '%s')" 
               % (self.name, cmapid, self.props))
      db.cursor.execute(query)
      self.id = db.cursor.lastrowid
      if self.cmap:
        db.cursor.execute(("insert into object_colourmap (object_id, colourmap_id, data_type) "
                           "values (%d, %d, %d)" % (self.id, self.cmap.id, Data.ColourValue)))

    data = struct.pack('f'*len(self.vertices), *self.vertices)
    
    size = 3
    count = len(self.vertices)
    width = 0
    minimum = maximum = 0 #Data value range

    db.insertGeometry(self, Data.Vertex, size, count, width, minimum, maximum, self.bmin, self.bmax, data)

    if len(self.colours):
        cdata = struct.pack('I'*len(self.colours), *self.colours)
        db.insertGeometry(self, Data.RGBA, 1, len(self.colours), 0, 0, 0, None, None, cdata)

    if len(self.values):
        #print "Values: " + str(self.vmin) + " to " + str(self.vmax)
        cvdata = struct.pack('f'*len(self.values), *self.values)
        db.insertGeometry(self, Data.ColourValue, 1, len(self.values), 0, self.vmin, self.vmax, None, None, cvdata)

    if len(self.widths):
        data = struct.pack('f'*len(self.widths), *self.widths)
        db.insertGeometry(self, Data.XWidth, 1, len(self.widths), 0, 0, 0, None, None, data)
    if len(self.heights):
        data = struct.pack('f'*len(self.heights), *self.heights)
        db.insertGeometry(self, Data.YHeight, 1, len(self.heights), 0, 0, 0, None, None, data)
    if len(self.lengths):
        data = struct.pack('f'*len(self.lengths), *self.lengths)
        db.insertGeometry(self, Data.ZLength, 1, len(self.lengths), 0, 0, 0, None, None, data)

    #Reset
    self.clear()

  def clear(self):
    #Reset/clear
    self.vertices = []
    self.colours = []
    self.values = []
    self.widths = self.heights = self.lengths = []

    self.bmin = [float("inf"),float("inf"),float("inf")]
    self.bmax = [float("-inf"),float("-inf"),float("-inf")]

    self.vmin = float("inf")
    self.vmax = float("-inf")

class Points(DrawingObject):
  Blur = 0
  Circle = 1
  Sphere = 2
  ShinySphere = 3
  Square = 4

  otype = Type.Point

  def __init__(self, name, cmap, size, pointtype, props=""):
    props = 'pointsize=%d\npointtype=%d\n' % (size, pointtype) + props
    super(Points, self).__init__(name, cmap, props)

class Lines(DrawingObject):
  otype = Type.Line

  def __init__(self, name, cmap, width, link=False, flat=False, props=""):
    props = 'lineWidth=%d\nlink=%d\nflat=%d' % (width, link, flat) + props
    super(Lines, self).__init__(name, cmap, props)

class Tracers(DrawingObject):
  otype = Type.Tracer

  def __init__(self, name, cmap, flat=False, scaling=1.0, props=""):
    props = 'flat=%d\nlinewidth=1\nlit=1\nscaling=%d\narrowhead=2\nsteps=0\ntime=0\n' % (flat, scaling) + props
    super(Tracers, self).__init__(name, cmap, props)

class Shapes(DrawingObject):
  Sphere = 0
  Cube = 1

  otype = Type.Shape

  def __init__(self, name, cmap, size=0.1, shape=Sphere, props=""):
    self.size = size
    props = 'shape=%d\n' % (shape) + props
    super(Shapes, self).__init__(name, cmap, props)

  def write(self, db):
    #Some default width/height/depth values if none provided
    if len(self.widths) == 0:
      self.widths = [self.size] * len(self.vertices)
    if len(self.heights) == 0:
      self.heights = [self.size] * len(self.vertices)
    if len(self.lengths) == 0:
      self.lengths = [self.size] * len(self.vertices)
    super(Shapes, self).write(db)

