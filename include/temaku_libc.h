#ifndef TEMAKU_LIBC_H
#define TEMAKU_LIBC_H

#include <stdio.h>

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

#endif /* TEMAKU_LIBC_H */
