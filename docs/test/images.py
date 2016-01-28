##  Copyright (C), 2010, Monash University
##  Copyright (C), 2010, Victorian Partnership for Advanced Computing (VPAC)
##  
##  This file is part of the CREDO library.
##  Developed as part of the Simulation, Analysis, Modelling program of 
##  AuScope Limited, and funded by the Australian Federal Government's
##  National Collaborative Research Infrastructure Strategy (NCRIS) program.
##
##  This library is free software; you can redistribute it and/or
##  modify it under the terms of the GNU Lesser General Public
##  License as published by the Free Software Foundation; either
##  version 2.1 of the License, or (at your option) any later version.
##
##  This library is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License along with this library; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
##  MA  02110-1301  USA

"""Utilities for basic image analysis and testing in relation to CREDO.
Original image test comparison scripts contributed by Owen Kaluza.
You will need the Python Imaging Library (PIL) installed to use."""

import sys
from math import sqrt
from math import fabs
try:
    import Image
    import ImageChops
except ImportError:
    from PIL import Image
    from PIL import ImageChops

def normalise(array, maxval):
   norm = [float(x) / maxval for x in array]
   return norm

def channelDiff(channel, hist1, hist2, pixels, bins):
    """Calculates histogram difference in one colour channel"""
    offset = channel * 256
    #Divide into bins
    chan1 = [0] * bins
    chan2 = [0] * bins
    binrange = 256 / bins
    for x in range(256):
        bin = x / binrange
        #Prevent out of range due to rounding
        if (bin == bins): bin = bins-1
        chan1[bin] += hist1[x+offset]
        chan2[bin] += hist2[x+offset]

    #Subtract histograms to get distance vectors
    dist = [a - b for a, b in zip(chan1, chan2)]
    #Normalise values to [-1,1]
    dist = normalise(dist, pixels)

    #Sum distances and return euclidean distance between
    return sqrt(sum([x*x for x in (dist)])) / sqrt(2)

def luminanceDiff(img1, img2):
    """Calculate image difference by luminance histogram.

    :arg img1: open PIL image
    :arg img2: open PIL image
    :returns: float representing difference between images
      in luminance histogram space
    """

    #Ensure images are in single channel Luminance format
    im1 = img1.convert('L')
    im2 = img2.convert('L')

    #Calculate image difference by colour histogram 
    width, height = im1.size
    pixels = width * height
    hist1 = im1.histogram()
    hist2 = im2.histogram()

    return channelDiff(0, hist1, hist2, pixels, 16)

def colourDiff(img1, img2):
    """Calculate image difference by colour histogram.

    :arg img1: open PIL image
    :arg img2: open PIL image
    :returns: float representing difference between images
      by colour histogram
    """

    #Ensure images are in 3 colour channel RGB format
    im1 = img1.convert('RGB')
    im2 = img2.convert('RGB')

    #Calculate image difference by colour histogram 
    width, height = im1.size
    pixels = width * height
    hist1 = im1.histogram()
    hist2 = im2.histogram()

    #Average result from several different bin sizes
    #This reduces false negatives from the pathological case where
    #two images with large areas of similar colour differ over a bin boundary
    rsum = gsum = bsum = 0
    count = 0
    for bins in range(8,16):
        #Get diff for each RGB channel
        rsum += channelDiff(0, hist1, hist2, pixels, bins)
        gsum += channelDiff(1, hist1, hist2, pixels, bins)
        bsum += channelDiff(2, hist1, hist2, pixels, bins)
        count += 3

    #Average and return
    return (rsum + gsum + bsum) / count

def pixelDiff20x20(img1, img2):
    """Compare two open images on a 20x20 basis."""
    #Simple check for transforms: resize to 20x20 and compare pixel by pixel
    img1b = img1.resize((20, 20), Image.ANTIALIAS)
    img2b = img2.resize((20, 20), Image.ANTIALIAS)

    return pixelDiff(img1b, img2b)

def pixelDiff(img1, img2):
    """Compare two open images on a pixel by pixel basis."""
    width, height = img1.size
    pixels = width * height

    #This subtracts each pixel value from the other
    diff = ImageChops.difference(img1, img2)
    #Create an image vector
    components = []
    pix = diff.load()
    width, height = diff.size
    for x in range(width):
        for y in range(height):
            for z in pix[x,y]:
                components.append(z)

    #Calculate euclidean distance 
    dist = sqrt(sum([x*x for x in (components)]))

    #Scale to [0,1] by dividing by maximum possible difference
    n = len(img1.getbands())
    maxdist = sqrt(pixels * n * (255*255))   #pixels * components * (255 possible values)^2
    return dist / maxdist

def compare(imgFilename1, imgFilename2, verbose=False):
    """Compare two image files.

    :returns: A tuple containing the diffs for each component
      (colour space, 400 pixel subsample)."""
    img1 = Image.open(imgFilename1)
    img2 = Image.open(imgFilename2)
    #Check size and components match
    if img1.size != img2.size or img1.getbands() != img2.getbands():
        return False
    #Colour comparison
    dist1 = colourDiff(img1, img2)
    if verbose: print "Colour space difference: %f" % dist1
    #Colour compare is not sensitive to flip/rotate so do 
    #a simple pixel by pixel compare as well
    dist2 = pixelDiff20x20(img1, img2)
    if verbose: print "Difference on 400 pixel subsample: %f" % dist2
    #Test fails if either value outside tolerance
    return dist1, dist2


if __name__ == "__main__":
    #Example usage, compare two images passed on command line
    diffs = compare(sys.argv[1], sys.argv[2])
    print diffs
