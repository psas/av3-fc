#!/usr/bin/env python
import os
import sys

def main():
  os.fsync()	# Clean any opened files
  pid = os.fork()
  time = da
  if pid > 0:
    os.execve(sys.argv[1])
  


if __name__=="__main__":
  main()
