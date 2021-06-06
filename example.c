#include <stdio.h>
#include <string.h>

#include <temaku.h>
#include <temaku_libc.h>

const char usage[] =
"=USAGE\n"
"  _progname_ |--help|               You're looking at it!\n"
"  _progname_ |--version|            Print %F{blue}version%f and %F{BLUE}stuff%f\n"
"  _progname_ |--export-usage|       Showcase custom writers\n"
"  _progname_ |<file>|               Do the thing with the file\n"
"=OPTIONS\n"
"  |-florg|                        Enable the |florg| capability\n"
"  |-floop|                        Use |floop| /whenever possible/\n"
"  |-no-floop|                     Do *not* use |%Sfloop%s|\n"
"  |-red|                          It's %F{red}red%f!\n"
"  |-inverse|                      Enable %RInverse video%r!\n"
"=ABOUT\n"
"  See also %L{https://www.example.com/about}the website%l\n"
;

int main(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        temaku_markup(NULL, &temaku_stderr_writer, usage, 0);
    } else if (strcmp(argv[1], "--export-usage") == 0) {
        temaku_file_writer_t writer = temaku_file_writer_open("usage.txt", "wb");
        temaku_options_t options = TEMAKU_DEFAULT_OPTIONS;
        options.do_markup = false;
        temaku_markup(&options, &writer.writer, usage, 0);
    } else {
        temaku_options_t options = TEMAKU_DEFAULT_OPTIONS;
        options.do_color = false;
        options.do_links = false;
        temaku_markup(&options, &temaku_stdout_writer, usage, 0);
    }
    return 0;
}
