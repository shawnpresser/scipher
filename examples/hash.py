import sys
import string
import random
import os
import subprocess

def syscmd(cmd, input=None):
    proc = subprocess.Popen(cmd,
            shell=True,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate(input=input)
    if stdout == None:
        stdout = ""
    if stderr != None and len(stderr) > 0:
        raise Exception(stderr)
    return stdout

def rand_string(size=10, chars=string.ascii_lowercase + string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for x in range(size))

def tmpname():
    return '/tmp/py.' + rand_string(32)

def tmpfile():
    name = tmpname()
    return open(name, 'w'), name

def encode(pw, msg):
    f, fname = tmpfile()
    try:
        f.write(msg)
        f.close()
        return syscmd("scipher enc "+fname, pw)
    finally:
        os.remove(fname)


def decode(pw, msg):
    f, fname = tmpfile()
    try:
        f.write(msg)
        f.close()
        return syscmd("scipher dec "+fname, pw)
    finally:
        os.remove(fname)



if sys.argv[1] == 'enc':
    msg = ""
    if not sys.stdin.isatty():
        msg = sys.stdin.read()
    sys.stdout.write(encode(sys.argv[2], msg))
elif sys.argv[1] == 'dec':
    sys.stdout.write(decode(sys.argv[2], sys.stdin.read()))
else:
    print "Usage: python hash.py {enc | dec} password"

