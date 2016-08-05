/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#include <archive.h>
#include <archive_entry.h>

#include <SDL.h>

#include "list.h"
#include "frame.h"

#include "archive.h"

char *
read_archive_entry(struct archive *a, struct archive_entry *entry, OUT size_t *totalsize)
{
    size_t  total, len_to_read;
    ssize_t size;
    char    buf[4096];
    char   *extracted_file;

    printf("archive: reading %s\n", archive_entry_pathname(entry));
    total = (size_t)archive_entry_size(entry);

    if(total < sizeof(buf)) {
        len_to_read = total;
    } else {
        len_to_read = sizeof(buf);
    }

    extracted_file = (char*)malloc(total);
    char *pos = extracted_file;

    for(;;) {
       size = archive_read_data(a, buf, len_to_read);

       memcpy(pos, buf, (size_t)size);
       pos += size;

       if((size_t)size < sizeof(buf)) {
            break;
        }
    }

    assert(pos - total == extracted_file);
    if(totalsize != NULL) {
        *totalsize = total;
    }

    return extracted_file;
}


Node *
read_whole_archive(IN char *filename, OUT char **json, OUT size_t *json_size)
{
    struct archive *a;
    struct archive_entry *entry;

    Node  *current_node = NULL, *start_node = NULL;
    Frame *frame;

    const char *ename;

    int ret = ARCHIVE_FAILED;

    if((a = archive_read_new()) == NULL) {
        fprintf(stderr, "archive_read_new() failed: %d\n", ret);
        return NULL;
    }

    ret = archive_read_support_format_zip(a);

    if(ret != ARCHIVE_OK) {
        fprintf(stderr, "archive_read_support_format_zip() failed: %d\n", ret);
        archive_read_free(a);
        return NULL;
    }

    ret = archive_read_open_filename(a, filename, 10240);

    if(ret != ARCHIVE_OK) {
        fprintf(stderr, "archive_read_open_filename() failed: %d\n", ret);
        archive_read_free(a);
        return NULL;
    }

    while(archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        ename = archive_entry_pathname(entry);

        if(strcmp(ename, "animation.json") == 0) {
            *json = read_archive_entry(a, entry, json_size);
            continue;
        }

        frame = frame_create();
        strcpy(frame->filename, ename);
        frame->image = read_archive_entry(a, entry, &frame->image_size);

        if(!start_node) {
            start_node = list_create((Frame*)frame);
            current_node = start_node;
        } else {
            assert(current_node != NULL);
            assert(current_node->next == NULL);
            current_node = list_insert_after(current_node, (Frame*)frame);
        }       
    }

    archive_read_free(a);
    return start_node;
}
