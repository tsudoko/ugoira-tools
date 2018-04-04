/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#include <miniz.h>
#include <SDL.h>

#include "list.h"
#include "frame.h"

#include "archive.h"

Node *
read_whole_archive(char *filename)
{
    mz_zip_archive z;
    unsigned int n;

    Node  *current_node = NULL, *start_node = NULL;
    Frame *frame;

    mz_zip_zero_struct(&z);
    if(!mz_zip_reader_init_file(&z, filename, 0)) {
        fprintf(stderr, "failed to open %s: %s\n", filename,
                mz_zip_get_error_string(z.m_last_error));
        return NULL;
    }

    n = mz_zip_reader_get_num_files(&z);
    for(unsigned int i = 0; i < n; i++) {
        frame = frame_create();
        mz_zip_reader_get_filename(&z, i, (char *)&frame->filename, sizeof frame->filename);
        frame->image = mz_zip_reader_extract_to_heap(&z, i, &frame->image_size, 0);
        if(frame->image == NULL) {
            fprintf(stderr, "failed to load %s: %s\n", frame->filename,
                    mz_zip_get_error_string(z.m_last_error));
            return NULL;
        }

        if(!start_node) {
            start_node = list_create((Frame*)frame);
            current_node = start_node;
        } else {
            assert(current_node != NULL);
            assert(current_node->next == NULL);
            current_node = list_insert_after(current_node, (Frame*)frame);
        }       
    }

    mz_zip_end(&z);
    return start_node;
}
