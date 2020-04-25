#!/usr/bin/python3

import argparse

ifile = ""
ofile = ""

def parse_config_macro(file, line):

	# Every configuration needs to contain
	# an assignment of some sort to be valid
	if '=' not in line:
		return

	# Ignore comment lines from config file
	if line[0] == '#':
		return

	# Split the line in its respective parts
	name, value = line.rstrip().split('=')

	# The configuration does not exist, for 
	# reasons we add it as a comment
	if value == 'y':
		file.write("CONFIG_%s=y\n" % name)

	# The configuration is active, no value given
	elif value == 'm':
		file.write("CONFIG_%s=m\n" % name)

	# Everything else is simply defined with a value
	else:
		file.write("CONFIG_%s=%s\n" % (name, value))


parser = argparse.ArgumentParser(description="elfboot auto.conf")
parser.add_argument('-i', '--ifile', required=True)
args = parser.parse_args()

if args.ifile:
	ifile = args.ifile

print("  BUILTIN %s" % "auto.conf")

with open(ifile, "r") as config, open("auto.conf", "w+") as autoconf:
	lines = config.readlines()

	autoconf.write("# This file is automatically created with:\n")
	autoconf.write("# builtin = $(PY) tools/builtin.py -i $(1)\n")
	autoconf.write("#\n# Do NOT change this file manually!\n\n")

	for line in lines:
		parse_config_macro(autoconf, line)