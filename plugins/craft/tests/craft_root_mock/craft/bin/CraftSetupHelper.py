import os
import sys

root = os.path.realpath(os.path.dirname(os.path.realpath(__file__)) + "/../../")

if "--getenv" in sys.argv:
    print("KDEROOT=" + root)
    print("PYTHONPATH=" + root + "/lib/site-packages")
    print("PATH=" + root + "/bin:" + os.environ["PATH"])
