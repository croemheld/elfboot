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
	if value == 'n':
		file.write("/* CONFIG_%s is not defined */\n" % name)

	# The configuration is active, no value given
	elif value == 'y':
		file.write("#define CONFIG_%s\n" % name)

	# Everything else is simply defined with a value
	else:
		file.write("#define CONFIG_%s %s\n" % (name, value))


parser = argparse.ArgumentParser(description="elfboot genconf")
parser.add_argument('-i', '--ifile', required=True)
parser.add_argument('-o', '--ofile', required=True)
args = parser.parse_args()

if args.ifile:
	ifile = args.ifile
if args.ofile:
	ofile = args.ofile

print("  GENCONF %s" % ofile)

with open(ifile, "r") as config, open(ofile, "w+") as header:
	lines = config.readlines()

	header.write("#ifndef __ELFBOOT_CONFIG_H__\n")
	header.write("#define __ELFBOOT_CONFIG_H__\n\n")

	for line in lines:
		parse_config_macro(header, line)

	header.write("\n#endif /* __ELFBOOT_CONFIG_H__ */\n")