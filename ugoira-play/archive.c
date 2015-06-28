#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <archive.h>
#include <archive_entry.h>

#include "list.h"
#include "archive.h"

Node* read_whole_archive(char *filename)
{
    struct archive *a;
    struct archive_entry *entry;

    size_t  total, len_to_read;
    ssize_t size;
    char    buf[4096];

    FILE *extracted_file;
    Node *current_node = NULL, *start_node = NULL;

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

        extracted_file = fmemopen(NULL, total, "r+");

        if(!start_node) {
            start_node = list_create((FILE*)extracted_file);
            current_node = start_node;
        } else {
            assert(current_node != NULL);
            current_node = list_insert_after(current_node, (FILE*)extracted_file);
        }

        for(;;) {
            size = archive_read_data(a, buf, len_to_read);

            //fprintf(stderr, "read %zu bytes\n", size);
            //fwrite(buf, (size_t)size, 1, stdout);
            fwrite(buf, (size_t)size, 1, extracted_file);

            if((size_t)size < sizeof(buf)) {
                //fprintf(stderr, "read %s (%zd)\n",
                //        archive_entry_pathname(entry), total);
                break;
            }
        }
    }

    archive_read_free(a);
    return start_node;
}
