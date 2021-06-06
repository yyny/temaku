#ifndef TEMAKU_LIBC_H
#define TEMAKU_LIBC_H

#include <stdio.h>

typedef struct temaku_file_writer temaku_file_writer_t;

struct temaku_file_writer {
    temaku_writer_t writer;
    FILE *fp;
};

static int temaku_file_writer_cb(TEMAKU_SELF *self, const void *data, size_t size)
{
    temaku_file_writer_t *writer = (temaku_file_writer_t *)self;
    return fwrite(data, sizeof(char), size, writer->fp);
}
static int temaku_stdout_writer_cb(TEMAKU_SELF *self, const void *data, size_t size)
{
    return fwrite(data, sizeof(char), size, stdout);
}
static int temaku_stderr_writer_cb(TEMAKU_SELF *self, const void *data, size_t size)
{
    return fwrite(data, sizeof(char), size, stderr);
}

temaku_writer_t temaku_stdout_writer = temaku_stdout_writer_cb;
temaku_writer_t temaku_stderr_writer = temaku_stderr_writer_cb;

static temaku_file_writer_t temaku_file_writer_new(FILE *fp)
{
    temaku_file_writer_t writer;
    writer.writer = temaku_file_writer_cb;
    writer.fp = fp;
    return writer;
}
static temaku_file_writer_t temaku_file_writer_open(const char *path, const char *mode)
{
    return temaku_file_writer_new(fopen(path, mode));
}

#endif /* TEMAKU_LIBC_H */
