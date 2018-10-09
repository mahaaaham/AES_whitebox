import os
import sys
from getopt import gnu_getopt, GetoptError
from random import randint, seed
from subprocess import CalledProcessError, check_call

NB = 4
AES_DIR = "../whitebox_aes/"
BINARY = os.path.basename (__file__)
BINARY_DIR = os.path.dirname (sys.argv[0])
if BINARY_DIR != "":
    BINARY_DIR += "/"

# Random key file generation function
def generate_key (filename):
    file = open (filename, "w")
    seed()
    for j in range (0, NB):
        for i in range (0, 4):
            file.write (hex (randint (0, 255))[2:].zfill (2) + " ")
        file.write ("  ")
    file.close()

# Parse options
try:
    options, remainder = gnu_getopt (sys.argv[1:], "ehmo:rvz",
                                     ["encodings", "help", "mixing-bijections",
                                      "zero-protection", "output=",
                                      "random", "verbose"])
except GetoptError as inst:
    print BINARY + ": " + inst.args[0]
    sys.exit (1)

protections = []
output = False;
random = False;
verbose = False;

for opt, arg in options:
    if opt in ("-e", "--encodings"):
        protections += [opt]
    if opt in ("-h", "--help"):
        print """\
Usage: {binary} [-e] [-h] [-m] [-o FILE] [-r] [-v] [-z] KEY
Generate a whitebox aes cipher.

Hexadecimal format is expected for the KEY file.

  -e, --encodings               use encodings only (no mixing bijections) to protect tables
  -h, --help                    display this help
  -m, --mixing-bijections       use mixing bijections only (no encodings) to protect tables
  -o, --output=FILE             use FILE for the output binary
  -r, --random                  generate key randomly (overwriting KEY if needed)
  -v, --verbose                 activate verbose mode
  -z, --zero-protection         do not protect tables (no encodings, no mixing bijections)\
""".format (binary = BINARY)
        sys.exit (0)
    if opt in ("-m", "--mixing-bijections"):
        protections += [opt]
    if opt in ("-o", "--output"):
        output = True
        aes_filename = arg
    if opt in ("-r", "--random"):
        random = True
    if opt in ("-v", "--verbose"):
        verbose = True
    if opt in ("-z", "--zero-protection"):
        protections += [opt]

if len (remainder) != 1:
    sys.exit (BINARY + ": one argument expected.")

key = remainder[0]
    
if not output:
    aes_filename = "aes_" + os.path.basename (key)

if random:
    generate_key (key)
    
if not verbose:
    sys.stdout = open (os.devnull, "w")

if not os.path.isfile (BINARY_DIR + "generate_tables"):
    sys.exit (BINARY + ": cannot find binary 'generate_tables'.")
    
try:
    # Generate lookup tables
    command = ["./" + BINARY_DIR + "generate_tables"] + protections + [key]
    print " ".join (command)
    check_call (command)

    # Compile whitebox aes
    command = ["make", "-C", BINARY_DIR + AES_DIR + "src"]
    print "\n" + " ".join (command)
    check_call (command, stdout = sys.stdout)

    # Copy executable
    command = ["cp", BINARY_DIR + AES_DIR + "src/aes", aes_filename]
    print "\n" + " ".join (command)
    check_call (command)

except CalledProcessError:
    sys.exit (1)
