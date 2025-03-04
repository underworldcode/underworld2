#!/usr/bin/env python
import sys
from credo.systest import *

compareStep = 5;
print("Comparing image results every %d timesteps" % compareStep)

testSuite = SysTestSuite("gLucifer", "RegressionTests-Images")

# Tests with single image check
images2d = []
images2d.append("windowViewport.00000.png")
images2d.append("windowColourBar.00000.png")
images2d.append("windowMeshViewer.00000.png")
images2d.append("windowTexture.00000.png")
images2d.append("windowMultiViewports.00010.png");

# Tests where images is checked for every timestep
img2DCompsMulti = [ "windowContours",
   "windowEigenvectors",
   "windowFeVariableSurface",
   "windowHistoricalSwarmTrajectory",
   "windowIsosurface",
   "windowPlot",
   "windowScalarField",
   "windowScalarFieldOnMesh",
   "windowSwarmRGBColourViewer",
   "windowSwarmVectors",
   "windowSwarmViewer",
   "windowVectorArrows"]

steps = 10;
for imgComp in img2DCompsMulti:
    for t in range(0, steps+1, compareStep):
        images2d.append("%s.%05d.png" % (imgComp, t))

# 3D Tests with single image check
images3d = []
images3d.append("windowViewport.00000.png")
images3d.append("windowCrossSection.00000.png")
images3d.append("windowMeshViewer.00000.png")
images3d.append("windowTexture.00000.png")
images3d.append("windowMultiViewports.00010.png");

# Tests where images is checked for every timestep
img3DCompsMulti = [
   "windowContours",
   "windowEigenvectorsCrossSection",
   "windowEigenvectors",
   "windowFieldSampler",
   "windowHistoricalSwarmTrajectory",
   "windowIsosurface",
   "windowScalarFieldCrossSection",
   "windowScalarFieldOnMeshCrossSection",
   "windowScalarField",
   "windowScalarFieldOnMesh",
   "windowSwarmRGBColourViewer",
   "windowSwarmShapes",
   "windowSwarmVectors",
   "windowSwarmViewer",
   "windowVectorArrowCrossSection",
   "windowVectorArrows"]

steps = 10;
for imgComp in img3DCompsMulti:
    for t in range(0, steps+1, compareStep):
        images3d.append("%s.%05d.png" % (imgComp, t))

# Single proc and two proc runs testing all objects in 2d and 3d
for np in [1, 2]:
    testSuite.addStdTest(ImageReferenceTest, ["testDrawingObjects2d.xml"],
        imagesToTest=images2d, defImageTol=(0.1, 0.05), 
        runSteps=steps, compareEvery=compareStep, nproc=np, expPathPrefix="expected")

    testSuite.addStdTest(ImageReferenceTest, ["testDrawingObjects3d.xml"],
        imagesToTest=images3d, defImageTol=(0.1, 0.05), 
        runSteps=steps, compareEvery=compareStep, nproc=np, expPathPrefix="expected")

# Set timeout 
testSuite.setAllTimeouts(minutes=10)

def suite():
    return testSuite

if __name__ == "__main__":
    testRunner = SysTestRunner()
    testRunner.runSuite(testSuite)
    #for sysTest in testSuite.sysTests:
    #    sysTest.regenerateFixture(jobRunner)
