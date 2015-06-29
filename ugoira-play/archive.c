/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <archive.h>
#include <archive_entry.h>

#include <SDL.h>

#include "list.h"
#include "archive.h"

Node* read_whole_archive(char *filename)
{
    struct archive *a;
    struct archive_entry *entry;

    size_t  total, len_to_read;
    ssize_t size;
    char    buf[4096];

    char      *extracted_file;
    SDL_RWops *rwops;
    Node      *current_node = NULL, *start_node = NULL;

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

            //fprintf(stderr, "read %zu bytes\n", size);
            memcpy(pos, buf, (size_t)size);
            pos += size;

            if((size_t)size < sizeof(buf)) {
                //fprintf(stderr, "read %s (%zd)\n",
                //        archive_entry_pathname(entry), total);
                break;
            }
        }

        rwops = SDL_RWFromMem(extracted_file, total);

        assert(pos - total == extracted_file);
        //free(extracted_file);

        if(!start_node) {
            start_node = list_create((SDL_RWops*)rwops);
            current_node = start_node;
        } else {
            assert(current_node != NULL);
            //assert(current_node->next == NULL);
            current_node = list_insert_after(current_node, (SDL_RWops*)rwops);
        }       
    }

    archive_read_free(a);
    return start_node;
}
