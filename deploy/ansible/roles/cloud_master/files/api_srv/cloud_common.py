# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Common functions and consts that are often used by other scripts in 
this directory"""

import subprocess
import sys
import time
import os
import shutil

# change me before the game
ROUTER_HOST = "127.0.0.1"

SSH_OPTS = [
    "-o", "StrictHostKeyChecking=yes",
    "-o", "CheckHostIP=yes",
    "-o", "NoHostAuthenticationForLocalhost=yes",
    "-o", "BatchMode=yes",
    "-o", "LogLevel=ERROR",
    "-o", "UserKnownHostsFile=known_hosts",
    "-o", "ConnectTimeout=10"
]


SSH_CLOUD_OPTS = SSH_OPTS + [
    "-o", "User=cloud",
    "-o", "IdentityFile=proctf2019_cloud_deploy"
]

DB_PATH = "/cloud/backend/db"

def get_cloud_ip(team):
    try:
        return open("db/team%d/cloud_ip" % team).read().strip()
    except FileNotFoundError as e:
        return None


def log_progress(*params):
    print("progress:", *params, flush=True)


def call_unitl_zero_exit(params, redirect_out_to_err=True, attempts=30, timeout=5):
    if redirect_out_to_err:
        stdout = sys.stderr
    else:
        stdout = sys.stdout

    for i in range(attempts-1):
        if subprocess.call(params, stdout=stdout) == 0:
            return True
        time.sleep(timeout)
    if subprocess.call(params, stdout=stdout) == 0:
        return True

    return None


def get_available_vms():
    try:
        ret = {}
        for line in open("%s/vms.txt" % DB_PATH):
            vm, vm_number = line.rsplit(maxsplit=1)
            ret[vm] = int(vm_number)
        return ret
    except (OSError, ValueError):
        return {}

def get_vm_name_by_num(num):
    if list(get_available_vms().values()).count(num) != 1:
        return ""

    for k, v in get_available_vms().items():
        if v == num:
            return k
    return ""
