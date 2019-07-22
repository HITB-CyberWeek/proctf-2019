#!/usr/bin/env python
import os
import re
import subprocess
import xml.etree.ElementTree as ET

# Usage:
# python build.py
#

regExp = re.compile(r'.+\s(.+)')
str = subprocess.check_output('mbed config -G GCC_ARM_PATH 2>/dev/null', shell=True)
gcc_arm = regExp.match(str).group(1)
print("GCC ARM Path: %s\n" % gcc_arm)
os.environ["PATH"] = os.environ["PATH"] + ":" + gcc_arm

os.system("mkdir BUILD; cd BUILD; rm -r *")

print("Build libpng")
if os.system("cd libpng; make") != 0:
    exit(1)
print("")

print("Build build_bmp")
if os.system("cd build_bmp; make") != 0:
    exit(1)
os.system("cp build_bmp/build_bmp BUILD/")
print("")

print("Build server")
os.system("mkdir BUILD/server")
if os.system("cd server; make") != 0:
    exit(1)
os.system("cp server/server BUILD/server/")
os.system("mkdir BUILD/server/data")
os.system("cp server/data/*.xml BUILD/server/data")
print("")

gamesXml = ET.ElementTree(file="server/data/games.xml").getroot()
for game in gamesXml:
	name = game.get("name")
	print("Build game: %s" % name)
	if os.system("cd server/data/%s; make" % name) != 0:
		exit(1)
	os.system("mkdir BUILD/server/data/%s" % name)
	os.system("cp server/data/%s/code.bin BUILD/server/data/%s/" % (name, name))
	os.system("cp server/data/%s/*.png BUILD/server/data/%s/" % (name, name))
	print("")

print("Build main_screen")
if os.system("cd main_screen; make") != 0:
    exit(1)
if os.system("cd main_screen; sudo ./make_fs.sh") != 0:
    exit(1)
os.system("cp main_screen/fs.bin BUILD/")
print("")

teamsXml = ET.ElementTree(file="server/data/teams.xml").getroot()
for team in teamsXml:
	name = team.get("name")
	print("Build firmware for team " + name)
	checksystemAuthKey = team.get("checksystemAuthKey")
	checksystemPort = team.get("checksystemPort")
	mac5 = team.get("mac5")
	
	team_data = "#define MAC5 %s\n" % mac5 
	team_data += "#define USER_NAME \"%s\"\n" % name
	team_data += "#define CHECKSYSTEM_PORT %s\n" % checksystemPort
	team_data += "#define CHECKSYSTEM_AUTH_KEY %s\n" % checksystemAuthKey 
	open("hw/team_data.h", "w").write(team_data)

	if os.system("cd hw; ./compile.sh --profile release") != 0:
		exit(1)
	os.system("mkdir BUILD/%s; cp hw/BUILD/DISCO_F746NG/GCC_ARM/hw.bin BUILD/%s/" % (name, name))
	os.system("cp hw/BUILD/DISCO_F746NG/GCC_ARM/hw.elf BUILD/%s/" % name)
	print("")

