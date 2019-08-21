#!/usr/bin/env python3

import sys
import traceback
from identity_server_http_client import IdentityServerHttpClient

OK, CORRUPT, MUMBLE, DOWN, CHECKER_ERROR = 101, 102, 103, 104, 110

def trace(message):
    print(message, file=sys.stderr)

def verdict(exit_code, public="", private=""):
    if public:
        print(public)
    if private:
        print(private, file=sys.stderr)
    sys.exit(exit_code)

def info():
    verdict(OK, "vulns: 1:2")

def check(args):
    if len(args) != 1:
        verdict(CHECKER_ERROR, "Wrong args count", "Wrong args count for check()")
    host = args[0]
    trace("check(%s)" % host)

    sys.exit(OK)

def put(args):
    if len(args) != 4:
        verdict(CHECKER_ERROR, "Wrong args count", "Wrong args count for put()")
    host, flag_id, flag_data, vuln = args
    trace("put(%s, %s, %s, %s)" % (host, flag_id, flag_data, vuln))

    if vuln == "1":
        sys.exit(OK)

    if vuln == "2":
        identity_server_client = IdentityServerHttpClient(host)
        # TODO generate random name
        result, feedback_queue = identity_server_client.create_new_user(flag_id.rstrip('='), flag_data.rstrip('='))
        if result != OK:
            sys.exit(result)

        print(feedback_queue)
        sys.exit(OK)

    verdict(CHECKER_ERROR, "Unknown vuln number for put()", "Unknown vuln number '%s' for put" % vuln)

def get(args):
    if len(args) != 4:
        verdict(CHECKER_ERROR, "Wrong args count", "Wrong args count for get()")
    host, flag_id, flag_data, vuln = args
    trace("get(%s, %s, %s, %s)" % (host, flag_id, flag_data, vuln))

    if vuln == "1":
        sys.exit(OK)

    if vuln == "2":
        sys.exit(OK)

    verdict(CHECKER_ERROR, "Unknown vuln number for get()", "Unknown vuln number '%s' for get" % vuln)

def main(args):
    if len(args) == 0:
        verdict(CHECKER_ERROR, "No args")
    try:
        if args[0] == "info":
            info()
        elif args[0] == "check":
            check(args[1:])
        elif args[0] == "put":
            put(args[1:])
        elif args[0] == "get":
            get(args[1:])
        else:
            verdict(CHECKER_ERROR, "Checker error", "Wrong action: " + args[0])
    except Exception as e:
        verdict(CHECKER_ERROR, "Checker error", "Exception: %s" % traceback.format_exc())

if __name__ == "__main__":
    try:
        main(sys.argv[1:])
        verdict(CHECKER_ERROR, "Checker error (NV)", "No verdict")
    except Exception as e:
        verdict(CHECKER_ERROR, "Checker error (CE)", "Exception: %s" % e)
