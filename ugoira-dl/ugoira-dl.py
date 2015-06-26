#!/usr/bin/env python3
from getopt import getopt
import urllib.request
import http.cookiejar
import os.path
import json
import sys
import re

SMALL_REGEX = "pixiv\.context\.ugokuIllustData            = (.*);pixiv"
LARGE_REGEX = "pixiv\.context\.ugokuIllustFullscreenData  = (.*\}\]\});"

verbose = False

def get_metadata(text, regex=SMALL_REGEX):
    metadata = re.search(regex, text);

    if metadata is None:
        raise Exception("couldn't find metadata")

    return json.loads(metadata.group(1))


def download_ugoira(url, cookies):
    if cookies:
        regex = LARGE_REGEX
    else:
        regex = SMALL_REGEX

    request = urllib.request.Request(url=url)
    cookies.add_cookie_header(request)

    with urllib.request.urlopen(request) as r:
        metadata = get_metadata(r.read().decode("utf-8"), regex)
        filename = metadata['src'].split('/')[-1]

        if verbose:
            print("dl", filename)

        request = urllib.request.Request(url=metadata['src'])
        request.add_header("Referer", url)
        #cookies.add_cookie_header(request)

        fr = urllib.request.urlopen(request)

        with open(filename, "wb") as f:
            while True:
                chunk = fr.read(1024)
                if not chunk: break
                f.write(chunk)
                f.flush()

        with open(filename.replace(".zip", ".json"), "w") as f:
            json.dump(metadata, f, indent=4)


def main():
    opts, args = getopt(sys.argv[1:], "vb:", ["verbose", "cookie-file="])

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
            if cookies:
                download_ugoira(arg, cookies)
            else:
                download_ugoira(arg)

if __name__ == "__main__":
    main()
