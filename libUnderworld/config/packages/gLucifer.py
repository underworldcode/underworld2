import os
from config import Package
from Underworld import Underworld

class gLucifer(Package):

    def setup_dependencies(self):
        with open(os.devnull, 'w') as FNULL:
            import subprocess
            subp = subprocess.Popen('python -c \'import lavavu\'', shell=True, stdout=FNULL, stderr=FNULL)
            if subp.wait() != 0:
                print("Unable to import lavavu from system python packages. Will try setup local copy.")
                self.pull_from_git( "https://github.com/OKaluza/LavaVu.git", "d99059e90dcb622df7164b306937872d912ba613", os.path.abspath('./gLucifer/Viewer'))


