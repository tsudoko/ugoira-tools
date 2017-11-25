#!/usr/bin/env python3

# This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
# Dedication license. Its contents can be found in the LICENSE file or at
# <http://creativecommons.org/publicdomain/zero/1.0/>

# XXX: not tested on non-jpg ugoira files, might need to reencode into a
#      different (lossless) format

from contextlib import contextmanager
import json
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile


@contextmanager
def cd(newdir):
    olddir = os.getcwd()
    os.chdir(os.path.expanduser(newdir))
    try:
        yield
    finally:
        os.chdir(olddir)


def ugoira2mkv(filename):
    with tempfile.TemporaryDirectory(prefix="ugoira2mkv") as d:
        frames = {}
        ffconcat = "ffconcat version 1.0\n"

        name = '.'.join(filename.split('.')[:-1])
        mkv_filename = os.path.basename(name) + ".mkv"

        with zipfile.ZipFile(filename) as f:
            f.extractall(d)

        with open(name + ".json") as f:
            frames = json.load(f)['frames']

        with cd(d):
            for i in frames:
                ffconcat += "file " + i['file'] + '\n'
                ffconcat += "duration " + str(i['delay'] / 1000) + '\n'

            with open("i.ffconcat", "w") as f:
                f.write(ffconcat)

            p = os.popen("ffmpeg -i i.ffconcat -codec copy " + mkv_filename)
            ret = p.close()

            if ret is not None:
                exit(ret)

        shutil.move(d + os.path.sep + mkv_filename, os.getcwd())


def main():
    if len(sys.argv) == 1:
        print("usage: %s file..." % os.path.basename(sys.argv[0]),
              file=sys.stderr)

    for i in sys.argv[1:]:
        ugoira2mkv(i)

if __name__ == "__main__":
    main()
