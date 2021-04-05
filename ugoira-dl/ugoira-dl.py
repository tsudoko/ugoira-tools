#!/usr/bin/env python3

# This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
# Dedication license. Its contents can be found in the LICENSE file or at
# <http://creativecommons.org/publicdomain/zero/1.0/>

from getpass import getpass
from getopt import getopt
import datetime
import hashlib
import json
import os.path
import re
import sys
import zipfile

import requests

CLIENT_ID = "MOBrBDS8blbauoSck0ZfDbtuzpyT"
CLIENT_SECRET = "lsACyCD94FhDUtGTXi3QzcFE2uU1hqtDaKeqrdwj"
TIME_SECRET = "28c1fdd170a5204386cb1313c7077b34f83e4aaf4aa829ce78c231e05b0bae2c"


def pixiv_login(*, username=None, password=None, refresh_token=None):
    timestamp = datetime.datetime.now(datetime.timezone.utc).isoformat(timespec='seconds')
    timehash = hashlib.md5(timestamp.encode() + TIME_SECRET.encode()).hexdigest()
    headers = {
        "x-client-time": timestamp,
        "x-client-hash": timehash,
    }
    assert (username and password) or refresh_token
    data = {
        "client_id": CLIENT_ID,
        "client_secret": CLIENT_SECRET,
    }

    if refresh_token is not None:
        data['grant_type'] = "refresh_token"
        data['refresh_token'] = refresh_token
    elif username is not None and password is not None:
        data['grant_type'] = "password"
        data['username'] = username
        data['password'] = password

    r = requests.post("https://oauth.secure.pixiv.net/auth/token", data=data, headers=headers)
    r = r.json()

    if "response" not in r or "access_token" not in r['response']:
        raise Exception("unexpected json:\n" + str(r))

    return r['response']['access_token']


def get_orig_frame_url(metadata):
    if not metadata.get("image_urls") or not metadata['image_urls'].get("large"):
        print("unexpected json (no ['image_urls']['large']):\n", metadata)
        return None

    url = metadata['image_urls']['large']

    return url.replace("_ugoira0.", "_ugoira{}.")


def dl_orig(metadata):
    headers = {"Referer": "https://public-api.secure.pixiv.net/"}
    filename_format = "%06d.%s"
    zipname = "{}_ugoira.zip".format(metadata['id'])

    url = get_orig_frame_url(metadata)

    if url is None:
        return None

    ext = url.split('.')[-1]
    frames = []

    for i in range(len(metadata['metadata']['frames'])):
        print("GET " + url.format(i))
        metadata['metadata']['frames'][i]['file'] = filename_format % (i, ext)

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


def dl_zip(metadata, size="600x600"):
    if not metadata['metadata'].get("zip_urls") or len(metadata['metadata']['zip_urls']) < 0:
        print("unexpected json (no ['metdata']['zip_urls']):\n", metadata)
        return None

    url = metadata['metadata']['zip_urls'][0]
    url = re.sub("ugoira[0-9]+x[0-9]", "ugoira" + size, url)

    headers = {"Referer": url}
    filename = os.path.basename(url)

    print("GET", url)
    r = requests.get(url, headers=headers, stream=True)

    if r.response_code != 200:
        print("unexpected response:", r.text)
        return None

    with open(filename, "wb") as f:
        for chunk in r.iter_content(chunk_size=4096):
            if chunk:
                f.write(chunk)
                f.flush()

    return filename


def dl(id_, token):
    headers = {"Authorization": "Bearer " + token}
    params = {"image_sizes": "large"}
    url = "https://public-api.secure.pixiv.net/v1/works/{}.json".format(id_)
    r = requests.get(url, headers=headers, params=params).json()

    if "response" not in r or len(r['response']) < 1:
        print("unexpected json:\n" + str(r), file=sys.stderr)
        return False

    data = r['response'][0]
    metadata = {"frames": []}

    for i in data['metadata']['frames']:
        metadata['frames'].append({"delay": i['delay_msec']})

    filename = dl_orig(data)

    if filename:
        for j, i in enumerate(data['metadata']['frames']):
            metadata['frames'][j]['file'] = i['file']
    else:  # TODO: zips from pixiv (600x600, 1920x1080)
        return False

    metadata_filename = filename[:-4] + ".json"

    with open(metadata_filename, "w") as f:
        json.dump(metadata, f)

    return True


def main():
    OPTS_SHORT = "p:s:u:t:Uv"
    OPTS_LONG = ["password=", "username=", "token=", "refresh-token=",
                 "unattended"]
    opts, args = getopt(sys.argv[1:], OPTS_SHORT, OPTS_LONG)

    username = None
    password = None
    refresh_token = None
    token = None
    unattended = False

    for o, a in opts:
        if o in ("-t", "--token"):
            token = a
        elif o in ("--refresh-token"):
            refresh_token = a
        elif o in ("-u", "--username"):
            if token:
                print("Using an access token, ignoring username supplied from"
                      "the command line")
            else:
                username = a
        elif o in ("-p", "--password"):
            if token:
                print("Using an access token, ignoring password supplied from"
                      "the command line")
            else:
                password = a
        elif o in ("-U", "--unattended"):
            unattended = True

    if len(args) < 1:
        print("usage: %s [-t token] ID..." %
              os.path.basename(sys.argv[0]), file=sys.stderr)
    else:
        if refresh_token:
            token = pixiv_login(refresh_token=refresh_token)
        elif username and password:
            token = pixiv_login(username=username, password=password)
        elif not unattended and not token:
            if not refresh_token:
                refresh_token = getpass("Refresh token (not echoed): ")
            token = pixiv_login(refresh_token=refresh_token)

        for arg in args:
            if dl(arg, token):
                print("{} done".format(arg))
            else:
                print("{} failed".format(arg))

if __name__ == "__main__":
    main()
