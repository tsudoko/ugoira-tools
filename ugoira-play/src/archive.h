/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#define IN
#define OUT

Node *read_whole_archive(IN char *filename, OUT char **json, OUT size_t *json_size);
