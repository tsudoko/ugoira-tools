#!/usr/bin/env python3

# This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
# Dedication license. Its contents can be found in the LICENSE file or at
# <http://creativecommons.org/publicdomain/zero/1.0/>

from getpass import getpass
from getopt import getopt
import http.cookiejar
import json
import os.path
import re
import sys

import requests

SMALL_REGEX = "pixiv\.context\.ugokuIllustData            = (.*);pixiv"
SMALL_REGEX_NOLOGIN = "pixiv\.context\.ugokuIllustData  = (.*\}\]\});"
LARGE_REGEX = "pixiv\.context\.ugokuIllustFullscreenData  = (.*\}\]\});"


def save_cookies(cookies, filename):
    cookiejar = http.cookiejar.MozillaCookieJar(filename)
    for c in cookies:
        args = dict(vars(c).items())
        args['rest'] = args['_rest']
        del args['_rest']

        c = http.cookiejar.Cookie(**args)
        cookiejar.set_cookie(c)
    cookiejar.save(filename)


def pixiv_login(username, password):
    s = requests.Session()

    data = {
        "mode": "login",
        "pixiv_id": username,
        "pass": password,
        "skip": 1,
    }

    s.post("https://www.secure.pixiv.net/login.php", data=data)

    return s.cookies


def get_metadata(text, regex=SMALL_REGEX_NOLOGIN):
    metadata = re.search(regex, text)

    if metadata is None:
        raise Exception("couldn't find metadata")

    return json.loads(metadata.group(1))


def download_ugoira(url, cookies=None):
    if cookies:
        regex = LARGE_REGEX
    else:
        regex = SMALL_REGEX_NOLOGIN

    r = requests.get(url, cookies=cookies)

    metadata = get_metadata(r.text, regex)
    filename = metadata['src'].split('/')[-1]

    if regex is not LARGE_REGEX:
        print("%s: downloading small version of %s" %
              (os.path.basename(sys.argv[0]), filename))

    print("dl", filename)

    r = requests.get(metadata['src'], headers={"Referer": url}, stream=True)

    with open(filename, "wb") as f:
        for chunk in r.iter_content(chunk_size=1024):
            if chunk:
                f.write(chunk)
                f.flush()

    with open(filename.replace(".zip", ".json"), "w") as f:
        json.dump(metadata, f, indent=4)


def main():
    OPTS_SHORT = "c:p:s:u:Uv"
    OPTS_LONG = ["cookie-jar=", "password=", "username=", "session-id=",
                 "unattended"]
    opts, args = getopt(sys.argv[1:], OPTS_SHORT, OPTS_LONG)

    username = None
    password = None
    cookies = None
    cookie_output_filename = None
    unattended = False

    for o, a in opts:
        if o in ("-s", "--session-id"):
            cookies = {"PHPSESSID": a}
        elif o in ("-c", "--cookie-jar"):
            cookie_output_filename = a
        elif o in ("-u", "--username"):
            if cookies:
                print("Using PHPSESSID, ignoring username supplied from the"
                      "command line")
            else:
                username = a
        elif o in ("-p", "--password"):
            if cookies:
                print("Using PHPSESSID, ignoring password supplied from the"
                      "command line" % cookies.filename)
            else:
                password = a
        elif o in ("-U", "--unattended"):
            unattended = True

    if len(args) < 1:
        print("usage: %s [-s session_id] URL..." %
              os.path.basename(sys.argv[0]), file=sys.stderr)
    else:
        if username and password:
            cookies = pixiv_login(username, password)
        elif not unattended and not cookies:
            if not username:
                username = input("Pixiv ID: ")
            if not password:
                password = getpass("Password: ")
            cookies = pixiv_login(username, password)

        for arg in args:
            download_ugoira(arg, cookies)

        if (cookies and cookie_output_filename):
            save_cookies(cookies, cookie_output_filename)

if __name__ == "__main__":
    main()
