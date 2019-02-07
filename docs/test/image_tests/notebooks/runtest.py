#This test runs notebooks that produce image output and checks the images against expected output
# coding: utf-8
import underworld as uw
import glucifer
import os
import glob
import subprocess
import shutil

#Viewer instance for running image tests
lv = glucifer.lavavu.Viewer(quality=1)

#files = glob.glob("*.ipynb")
nbdir = "../../../user_guide/"
files = glob.glob(nbdir  + "*.ipynb")
#files = [
#    '01_GettingStarted.ipynb', 
#    '02_TheMesh.ipynb', 
#    '03_MeshVariable.ipynb', 
#    '04_Swarms.ipynb', 
#    '07_Utilities.ipynb', 
#    '08_Visualisation.ipynb',
#    ]
#Save working dir
wd = os.getcwd()

#Process list of notebooks
for fname in files:
    #Get filename without path and extension
    notebook = os.path.splitext(os.path.basename(fname))[0]
    print("Testing Notebook: " + notebook)
    #Check if the test dir exists, if not create
    dirfound = os.path.exists(notebook)
    if not dirfound: 
        print("Creating dirs: " + notebook + '/expected')
        os.makedirs(os.path.join(notebook, 'expected'))
    #Create log file
    outName = notebook + "/convert.out"
    with open(outName, "w") as outFile:
        #Notebooks must be converted to py before running or images will be generated inline and not saved to disk
        subprocess.check_call(['jupyter', 'nbconvert', '--to', 'script', fname, '--output', os.path.join(wd,notebook, notebook)],
                stdout=outFile, stderr=outFile )
        #Change to working dir for test
        os.chdir(notebook)
        #Execute converted script
        subprocess.check_call(['ipython', notebook+".py"], stdout=outFile, stderr=outFile )

    #Use output of the initial run as expected data
    if not dirfound:
        print("Using files created by initial run as expected output for tests")
        images = glob.glob("*.png")
        for f in images:
            shutil.move(f, os.path.join('expected', f))
    else:
        #Check the image results
        if uw.rank() == 0:
            lv.testimages(tolerance=1e-3)

        # cleanup
        import glob, os
        for f in glob.glob("Figure_*"):
            os.remove(f)
    os.remove("convert.out")
    os.remove(notebook+".py")

    #Restore working dir
    os.chdir(wd)

