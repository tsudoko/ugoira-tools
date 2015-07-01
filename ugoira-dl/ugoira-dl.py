#!/usr/bin/env python3

# This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
# Dedication license. Its contents can be found in the LICENSE file or at
# <http://creativecommons.org/publicdomain/zero/1.0/>

from getopt import getopt
import http.cookiejar
import requests
import os.path
import json
import sys
import re

SMALL_REGEX = "pixiv\.context\.ugokuIllustData            = (.*);pixiv"
SMALL_REGEX_NOLOGIN = "pixiv\.context\.ugokuIllustData  = (.*\}\]\});"
LARGE_REGEX = "pixiv\.context\.ugokuIllustFullscreenData  = (.*\}\]\});"

verbose = False


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

    if verbose:
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
    opts, args = getopt(sys.argv[1:], "vb:", ["verbose", "cookie-file="])
    cookies = None

    for o, a in opts:
        if o in ("-b", "--cookie-file"):
            cookies = http.cookiejar.MozillaCookieJar(a)
            cookies.load()
        elif o in ("-v", "--verbose"):
            verbose = True

    if len(args) < 1:
        print("usage: %s [-b cookie_jar] URL..." %
              os.path.basename(sys.argv[0]), file=sys.stderr)
    else:
        for arg in args:
            download_ugoira(arg, cookies)

if __name__ == "__main__":
    main()
