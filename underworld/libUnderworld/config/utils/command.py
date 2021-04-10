import subprocess

def run(cmd_str):
    subp = subprocess.Popen(cmd_str,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            shell=True)
    stdout, stderr = subp.communicate()
    result = subp.wait()
    del subp
    return (result, stdout, stderr)
