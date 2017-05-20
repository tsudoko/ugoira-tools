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
import shutil
import sys
import zipfile

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


# TODO: R-18 support, requires API login
def get_orig_frame_url(id_):
    params = {"illust_id": id_}
    detail = requests.get("https://app-api.pixiv.net/v1/illust/detail",
                          params=params).json()

    if not detail.get("illust") \
    or not detail['illust'].get("meta_single_page") \
    or not detail['illust']['meta_single_page'].get("original_image_url"):
        print("%s: get_frames_api: unexpected json: %s" %
              (os.path.basename(sys.argv[0]), detail), file=sys.syderr)
        return None

    url = detail['illust']['meta_single_page']['original_image_url']
    return url.replace("_ugoira0.", "_ugoira{}.")


def dl_orig(id_, metadata):
    headers = {"Referer": "https://www.pixiv.net/"}
    filename_format = "%06d.%s"
    zipname = "{}_ugoiraorig.zip".format(id_)

    url = get_orig_frame_url(id_)

    if url is None:
        return None

    ext = url.split('.')[-1]
    frames = []

    for i in range(len(metadata['frames'])):
        print("GET " + url.format(i))

        r = requests.get(url.format(i), headers=headers)
        if r.status_code != 200:
            print("%s: error while downloading frame %d: %s" %
                  (os.path.basename(sys.argv[0]), i, r.text), file=sys.stderr)
            return None

        frames.append(r.content)

    with zipfile.ZipFile(zipname, "w") as f:
        for i, frame in enumerate(frames):
            f.writestr(filename_format % (i, ext), frame)

    return zipname


def dl_zip(src, regex, cookies=None):
    headers = {"Referer": "https://www.pixiv.net/"}

    filename = os.path.basename(src)

    if regex is not LARGE_REGEX:
        print("%s: downloading small version of %s" %
              (os.path.basename(sys.argv[0]), filename))

    print("GET", src)

    r = requests.get(src, headers=headers, stream=True)

    with open(filename, "wb") as f:
        for chunk in r.iter_content(chunk_size=4096):
            if chunk:
                f.write(chunk)
                f.flush()

    return filename


def get_metadata(text, regex=SMALL_REGEX_NOLOGIN):
    metadata = re.search(regex, text)

    if metadata is None:
        raise Exception("couldn't find metadata")

    return metadata.group(1)


def dl(url, cookies=None):
    if cookies:
        regex = LARGE_REGEX
    else:
        regex = SMALL_REGEX_NOLOGIN

    r = requests.get(url, cookies=cookies)

    metadata = get_metadata(r.text, regex)

    src = json.loads(metadata)['src']
    ugoname = src.split('/')[-1]

    filename = dl_orig(re.sub("_.*", "", ugoname), json.loads(metadata)) or \
               dl_zip(src, regex, cookies)

    metadata_filename = filename[:-4] + ".json"

    with open(metadata_filename, "w") as f:
        f.write(metadata)

    print(filename, "done")
    return filename, metadata_filename


def main():
    OPTS_SHORT = "c:p:s:u:Uv"
    OPTS_LONG = ["cookie-jar=", "password=", "username=", "session-id=",
                 "unattended", "keep-original"]
    opts, args = getopt(sys.argv[1:], OPTS_SHORT, OPTS_LONG)

    username = None
    password = None
    cookies = None
    cookie_output_filename = None
    unattended = False
    keep_original = False

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
            dl(arg, cookies)

        if (cookies and cookie_output_filename):
            save_cookies(cookies, cookie_output_filename)

if __name__ == "__main__":
    main()
