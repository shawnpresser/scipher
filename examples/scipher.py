import sys
import string
import random
import os
from subprocess import Popen, PIPE

def usage():
    print "                                                    "
    print "  python scipher.py enc [infile]                    "
    print "                                                    "
    print "    examples:                                       "
    print "                                                    "
    print "    encrypt (hash) a password:                      "
    print "    $ printf 'somepassword' > pwfile;               "
    print "    $ cat pwfile | python scipher.py enc > hashfile "
    print "                                                    "
    print "    decrypt (test) a password:                      "
    print "    $ printf 'somepassword' > pwfile;               "
    print "    $ cat pwfile | python scipher.py dec hashfile   "
    print "                                                    "
    print "    (if no output, then the password was correct)   "
    print "                                                    "
    print "    encrypt a file with a passphrase:               "
    print "    $ printf 'somepassword' > pwfile;               "
    print "    $ cat pwfile | python scipher.py enc photo.jpg > photo.enc"
    print "                                                    "
    print "    decrypt an encrypted file:                      "
    print "    $ printf 'somepassword' > pwfile;               "
    print "    $ cat pwfile | python scipher.py dec photo.enc > photo.jpg"
    print "                                                    "
    sys.exit(1)

def syscmd(cmd, input=None):
    proc = Popen(cmd,
            shell=True,
            stdin=PIPE,
            stdout=PIPE,
            stderr=PIPE)
    stdout, stderr = proc.communicate(input=input)
    if stdout == None:
        stdout = ""
    if stderr != None and len(stderr) > 0:
        raise Exception(stderr)
    return stdout

def encrypt(pw, msgfile):
    return syscmd("scipher enc "+msgfile, pw)

def decrypt(pw, msgfile):
    return syscmd("scipher dec "+msgfile, pw)

if len(sys.argv) < 2 or (sys.argv[1] == 'dec' and len(sys.argv) < 3):
    usage()

msgfile = ""
if len(sys.argv) >= 3:
    msgfile = sys.argv[2]

if sys.stdin.isatty():
    print "pipe a password into stdin"
    sys.exit(2)

pw = sys.stdin.read()

if sys.argv[1] == 'enc':
    sys.stdout.write(encrypt(pw, msgfile))
elif sys.argv[1] == 'dec':
    sys.stdout.write(decrypt(pw, msgfile))
else:
    usage()

