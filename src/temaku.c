#include <temaku.h>

#include <stdio.h>
#include <stdint.h>

// Undocumented symbols
TEMAKU_API(int) temaku_write_ansi_sequence_cb(TEMAKU_SELF *self, struct temaku_options *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg);
TEMAKU_API(int) temaku_write_html_sequence_cb(TEMAKU_SELF *self, struct temaku_options *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg);

static inline bool strequal(const char *a, const char *b)
{
    for (;;) {
        if (!*a && !*b) return true;
        if (*a != *b) return false;
        a++, b++;
    }
    return true;
}
static inline bool strequali(const char *a, const char *b)
{
    for (;;) {
        if (!*a && !*b) return true;
        if (tolower(*a) != tolower(*b)) return false;
        a++, b++;
    }
    return true;
}
static inline bool strequaln(const char *a, const char *b, size_t n)
{

    for (;;) {
        if (!n) return true;
        if (!*a && !*b) return true;
        if (*a != *b) return false;
        a++, b++, n--;
    }
    return true;
}
static inline bool strequalni(const char *a, const char *b, size_t n)
{

    for (;;) {
        if (!n) return true;
        if (!*a && !*b) return true;
        if (tolower(*a) != tolower(*b)) return false;
        a++, b++, n--;
    }
    return true;
}

static bool temaku_wordchar(struct temaku_options *options, const char *str)
{
    if (strchr(options->wordchars, '%') && str[0] == '%') return str[1] == '%';
    return strchr(options->wordchars, str[0]);
}

TEMAKU_FUN(int) temaku_write(temaku_writer_t *self, const void *data, size_t size)
{
    return (*self)((TEMAKU_SELF *)self, data, size);
}
TEMAKU_FUN(int) temaku_writestr(temaku_writer_t *self, const char *str)
{
    return temaku_write(self, str, strlen(str));
}
TEMAKU_FUN(int) temaku_writechar(temaku_writer_t *self, char c)
{
    return temaku_write(self, &c, 1);
}
TEMAKU_FUN(int) temaku_writeint(temaku_writer_t *self, int val)
{
    static char buffer[24];
    snprintf(buffer, sizeof(buffer), "%d", val);
    return temaku_writestr(self, buffer);
}
TEMAKU_FUN(int) temaku_writehex(temaku_writer_t *self, int val)
{
    static char buffer[24];
    snprintf(buffer, sizeof(buffer), "%x", val);
    return temaku_writestr(self, buffer);
}
TEMAKU_FUN(int) temaku_writeurl(temaku_writer_t *self, const char *url, size_t size)
{
    int nwritten = 0;
    for (size_t i=0; i < size; i++) {
        if (url[i] <= 0x20 || url[i] >= 0x7f || url[i] == '<' || url[i] == '>' || url[i] == '%' || url[i] == '"') {
            temaku_writechar(self, '%');
            nwritten += temaku_writehex(self, url[i]);
        } else {
            nwritten += temaku_writechar(self, url[i]);
        }
    }
    return nwritten;
}
TEMAKU_FUN(int) temaku_write_ansi_sequence_cb(TEMAKU_SELF *self, struct temaku_options *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg)
{
    static const char *fgcolors[16] = {
        "30", "31", "32", "33", "34", "35", "36", "37",
        "90", "91", "92", "93", "94", "95", "96", "97",
    };
    static const char *bgcolors[16] = {
        "40", "41", "42", "43", "44", "45", "46", "47",
        "100", "101", "102", "103", "104", "105", "106", "107",
    };
    int nwritten = 0;
    (void)self;
    switch (seq) {
    case TEMAKU_START: return 0;
    case TEMAKU_END: return 0;
    case TEMAKU_DATA:
        {
            struct temaku_string *data = (struct temaku_string *)arg;
            return temaku_write(writer, data->base, data->size);
        }
        break;
    case TEMAKU_HEADER_START: return temaku_writestr(writer, "\x1b[1;4m"); /* add 97 for bright white :) */
    case TEMAKU_HEADER_END: return temaku_writestr(writer, "\x1b[22;24m"); /* add 39 for bright white :) */
    case TEMAKU_BOLD_START: return temaku_writestr(writer, "\x1b[1m");
    case TEMAKU_BOLD_END: return temaku_writestr(writer, "\x1b[22m");
    case TEMAKU_ITALIC_START: return temaku_writestr(writer, "\x1b[3m");
    case TEMAKU_ITALIC_END: return temaku_writestr(writer, "\x1b[23m");
    case TEMAKU_UNDERLINE_START: return temaku_writestr(writer, "\x1b[4m");
    case TEMAKU_UNDERLINE_END: return temaku_writestr(writer, "\x1b[24m");
    case TEMAKU_STRIKETHROUGH_START: return temaku_writestr(writer, "\x1b[9m");
    case TEMAKU_STRIKETHROUGH_END: return temaku_writestr(writer, "\x1b[29m");
    case TEMAKU_REVERSE_VIDEO_START: return temaku_writestr(writer, "\x1b[7m");
    case TEMAKU_REVERSE_VIDEO_END: return temaku_writestr(writer, "\x1b[27m");
    case TEMAKU_ALTERNATIVE_START: return temaku_writestr(writer, "\x1b[2m");
    case TEMAKU_ALTERNATIVE_END: return temaku_writestr(writer, "\x1b[22m");
    case TEMAKU_FGCOLOR_START:
        {
            int fgcolor = *(int *)arg;
            if (fgcolor != -1) {
                nwritten += temaku_writestr(writer, "\x1b[");
                nwritten += temaku_writestr(writer, fgcolors[fgcolor % 16]);
                nwritten += temaku_writestr(writer, "m");
            }
        }
        break;
    case TEMAKU_FGCOLOR_END: return temaku_writestr(writer, "\x1b[39m");
    case TEMAKU_BGCOLOR_START:
        {
            int bgcolor = *(int *)arg;
            if (bgcolor != -1) {
                nwritten += temaku_writestr(writer, "\x1b[");
                nwritten += temaku_writestr(writer, bgcolors[bgcolor % 16]);
                nwritten += temaku_writestr(writer, "m");
            }
        }
        break;
    case TEMAKU_BGCOLOR_END: return temaku_writestr(writer, "\x1b[49m");
    case TEMAKU_BGLINE_START: return temaku_writestr(writer, "\x1b[K");
    case TEMAKU_BGLINE_END: return 0;
    case TEMAKU_LINK_START:
        {
            struct temaku_string *url = (struct temaku_string *)arg;
            nwritten += temaku_writestr(writer, "\x1b]8;;");
            nwritten += temaku_write(writer, url->base, url->size);
            nwritten += temaku_writestr(writer, options->string_terminator);
        }
        break;
    case TEMAKU_LINK_END:
        nwritten += temaku_writestr(writer, "\x1b]8;;");
        nwritten += temaku_writestr(writer, options->string_terminator);
        break;
    }
    return nwritten;
}
TEMAKU_FUN(int) temaku_write_html_sequence_cb(TEMAKU_SELF *self, struct temaku_options *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg)
{
    static const char *fgcolors[16] = {
        "#010101", // BLACK
        "#DE382B", // RED
        "#39B54A", // GREEN
        "#FFC706", // YELLOW
        "#006FB8", // BLUE
        "#762671", // PURPLE
        "#2CB5E9", // CYAN
        "#CCCCCC", // WHITE
        "#808080", // BLACK
        "#FF0000", // RED
        "#00FF00", // GREEN
        "#FFFF00", // YELLOW
        "#0000FF", // BLUE
        "#FF00FF", // PURPLE
        "#00FFFF", // CYAN
        "#FFFFFF", // WHITE
    };
    static const char *bgcolors[16] = {
        "#010101", // BLACK
        "#DE382B", // RED
        "#39B54A", // GREEN
        "#FFC706", // YELLOW
        "#006FB8", // BLUE
        "#762671", // PURPLE
        "#2CB5E9", // CYAN
        "#CCCCCC", // WHITE
        "#808080", // BLACK
        "#FF0000", // RED
        "#00FF00", // GREEN
        "#FFFF00", // YELLOW
        "#0000FF", // BLUE
        "#FF00FF", // PURPLE
        "#00FFFF", // CYAN
        "#FFFFFF", // WHITE
    };
    int nwritten = 0;
        (void)self;
    (void)options;
    switch (seq) {
    case TEMAKU_START: return temaku_writestr(writer, "<pre>");
    case TEMAKU_END: return temaku_writestr(writer, "</pre>");
    case TEMAKU_DATA:
        {
            struct temaku_string *data = (struct temaku_string *)arg;
            for (size_t i=0; i < data->size; i++) {
                char c = data->base[i];
                switch (c) {
                case '<':
                    nwritten += temaku_writestr(writer, "&lt;");
                    break;
                case '>':
                    nwritten += temaku_writestr(writer, "&gt;");
                    break;
                case '&':
                    nwritten += temaku_writestr(writer, "&amp;");
                    break;
                default:
                    nwritten += temaku_writechar(writer, c);
                    break;
                }
            }
        }
        break;
    case TEMAKU_HEADER_START: return temaku_writestr(writer, "<h1>");
    case TEMAKU_HEADER_END: return temaku_writestr(writer, "</h1>");
    case TEMAKU_BOLD_START: return temaku_writestr(writer, "<span style=\"font-weight:bold\">");
    case TEMAKU_BOLD_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_ITALIC_START: return temaku_writestr(writer, "<span style=\"font-style:italic\">");
    case TEMAKU_ITALIC_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_UNDERLINE_START: return temaku_writestr(writer, "<span style=\"text-decoration:underline\">");
    case TEMAKU_UNDERLINE_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_STRIKETHROUGH_START: return temaku_writestr(writer, "<span style=\"text-decoration:line-through\">");
    case TEMAKU_STRIKETHROUGH_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_REVERSE_VIDEO_START: return 0; /* not implemented */
    case TEMAKU_REVERSE_VIDEO_END: return 0; /* not implemented */
    case TEMAKU_ALTERNATIVE_START: return temaku_writestr(writer, "<span style=\"color:#404040\">");
    case TEMAKU_ALTERNATIVE_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_FGCOLOR_START:
        {
            int fgcolor = *(int *)arg;
            if (fgcolor != -1) {
                nwritten += temaku_writestr(writer, "<span style=\"color:");
                nwritten += temaku_writestr(writer, fgcolors[fgcolor % 16]);
                nwritten += temaku_writestr(writer, "\">");
            }
        }
        break;
    case TEMAKU_FGCOLOR_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_BGCOLOR_START:
        {
            int bgcolor = *(int *)arg;
            if (bgcolor != -1) {
                nwritten += temaku_writestr(writer, "<span style=\"background:");
                nwritten += temaku_writestr(writer, bgcolors[bgcolor % 16]);
                nwritten += temaku_writestr(writer, "\">");
            }
        }
        break;
    case TEMAKU_BGCOLOR_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_BGLINE_START:
        {
            int bgcolor = *(int *)arg;
            if (bgcolor != -1) {
                nwritten += temaku_writestr(writer, "<span style=\"background:");
                nwritten += temaku_writestr(writer, bgcolors[bgcolor % 16]);
                nwritten += temaku_writestr(writer, "\">");
            }
        }
        break;
    case TEMAKU_BGLINE_END: return temaku_writestr(writer, "</span>");
    case TEMAKU_LINK_START:
        {
            struct temaku_string *url = (struct temaku_string *)arg;
            nwritten += temaku_writestr(writer, "<a href=\"");
            nwritten += temaku_writeurl(writer, url->base, url->size);
            nwritten += temaku_writestr(writer, "\">");
        }
        break;
    case TEMAKU_LINK_END:
        nwritten += temaku_writestr(writer, "</a>");
        break;
    }
    return nwritten;
}

TEMAKU_VAR(struct temaku_options) temaku_default_options = TEMAKU_DEFAULT_OPTIONS;
TEMAKU_VAR(temaku_sequence_writer_t) temaku_write_ansi_sequence = &temaku_write_ansi_sequence_cb;
TEMAKU_VAR(temaku_sequence_writer_t) temaku_write_html_sequence = &temaku_write_html_sequence_cb;

TEMAKU_FUN(int) temaku_writesequence(struct temaku_options *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg)
{
    return (*options->sequence_writer)((TEMAKU_SELF*)options->sequence_writer, options, writer, seq, arg);
}
TEMAKU_FUN(int) temaku_markup(struct temaku_options *options, temaku_writer_t *writer, const char *markup, size_t markuplen)
{
#define TEMAKU_DO_COLOR(block) if (options->do_markup && options->do_color) do { block; } while (0)
#define TEMAKU_DO_STYLE(block) if (options->do_markup && options->do_style) do { block; } while (0)
#define TEMAKU_DO_LINKS(block) if (options->do_markup && options->do_links) do { block; } while (0)
    static const char *color_names[] = {
        "black",
        "red",
        "green",
        "yellow",
        "blue",
        "purple",
        "cyan",
        "white",
        NULL
    };
    int row = 0, column = 0;
    int fgcolor = -1;
    int bgcolor = -1;
    const char *s = markup;
    bool in_word = false;
    unsigned ctx = 0;
    struct temaku_string data = { NULL, 0 };
    if (markuplen == 0 || markuplen == SIZE_MAX) {
        markuplen = strlen(markup);
    }
    if (options == NULL) options = &temaku_default_options;
    enum {
        CTX_HEADER      = 0x1,
        CTX_BOLD        = 0x2,
        CTX_ITALIC      = 0x4,
        CTX_UNDERLINE   = 0x8,
        CTX_ALTERNATIVE = 0x10,
        CTX_BGLINE      = 0x20,
    };
    data.base = markup;
    data.size = markuplen;
    temaku_writesequence(options, writer, TEMAKU_START, &data);
    while (*s) {
        const char *seq = s;
        char c = *s;
        ++s;
        switch (c) {
        case '=':
            if (column != 0) goto put;
            ctx |= CTX_HEADER;
            TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_HEADER_START, NULL));
            break;
        case '*':
            if ((ctx & CTX_BOLD) == 0) {
                if (in_word) goto put;
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BOLD_START, NULL));
                ctx |= CTX_BOLD;
            } else if (!temaku_wordchar(options, s)) {
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BOLD_END, NULL));
                ctx &= ~CTX_BOLD;
            }
            break;
        case '/':
            if ((ctx & CTX_ITALIC) == 0) {
                if (in_word) goto put;
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ITALIC_START, NULL));
                ctx |= CTX_ITALIC;
            } else if (!temaku_wordchar(options, s)) {
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ITALIC_END, NULL));
                ctx &= ~CTX_ITALIC;
            }
            break;
        case '_':
            if ((ctx & CTX_UNDERLINE) == 0) {
                if (in_word) goto put;
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_UNDERLINE_START, NULL));
                ctx |= CTX_UNDERLINE;
            } else if (!temaku_wordchar(options, s)) {
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_UNDERLINE_END, NULL));
                ctx &= ~CTX_UNDERLINE;
            }
            break;
        case '|':
            if ((ctx & CTX_ALTERNATIVE) == 0) {
                if (in_word) goto put;
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ALTERNATIVE_START, NULL));
                ctx |= CTX_ALTERNATIVE;
            } else if (!temaku_wordchar(options, s)) {
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ALTERNATIVE_END, NULL));
                ctx &= ~CTX_ALTERNATIVE;
            }
            break;
        case '\n':
            data.base = &c;
            data.size = 1;
            temaku_writesequence(options, writer, TEMAKU_DATA, &data);
            if (ctx & CTX_HEADER)
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_HEADER_END, NULL));
            if (ctx & CTX_BOLD)
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BOLD_END, NULL));
            if (ctx & CTX_ITALIC)
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ITALIC_END, NULL));
            if (ctx & CTX_UNDERLINE)
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_UNDERLINE_END, NULL));
            if (ctx & CTX_ALTERNATIVE)
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ALTERNATIVE_END, NULL));
            if (ctx & CTX_BGLINE)
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BGLINE_END, NULL));
            ctx = 0;
            ++row;
            column = 0;
            in_word = false;
            break;
        case '%':
            c = *s;
            s += !!*s;
            switch (c) {
            case '\0': break;
            case '{':
                {
                    const char *start = s;
                    while (*s && (s[-1] != '}' || s[-2] != '%')) ++s;
                    int size = s - start - (*s?2:0);
                    temaku_write(writer, start, size);
                }
                break;
            case 'F':
            case 'K':
                if (*s != '{') break;
                ++s;
                const char *start = s;
                while (*s && *s != '}') ++s;
                int size = s - start;
                TEMAKU_DO_COLOR({
                    if (strequalni("reset", start, size)) {
                        if (c == 'F') {
                            TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_FGCOLOR_END, &fgcolor));
                            fgcolor = -1;
                        } else {
                            TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_BGCOLOR_END, &bgcolor));
                            bgcolor = -1;
                        }
                    } else for (int j=0; color_names[j]; j++) {
                        if (strequaln(color_names[j], start, size)) {
                            if (c == 'F') {
                                fgcolor = j;
                                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_FGCOLOR_START, &fgcolor));
                            } else {
                                bgcolor = j;
                                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_BGCOLOR_START, &fgcolor));
                            }
                            break;
                        } else if (strequalni(color_names[j], start, size)) {
                            /* Color name contains at least one uppercase letter */
                            if (c == 'F') {
                                fgcolor = j + 8; /* Use bright colors */
                                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_FGCOLOR_START, &fgcolor));
                            } else {
                                bgcolor = j + 8; /* Use bright colors */
                                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_BGCOLOR_START, &fgcolor));
                            }
                            break;
                        }
                    }
                });
                s += !!*s;
                break;
            case 'f':
                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_FGCOLOR_END, &fgcolor));
                fgcolor = -1;
                break;
            case 'k':
                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_BGCOLOR_END, &bgcolor));
                bgcolor = -1;
                break;
            case 'L':
                if (*s != '{') break;
                ++s;
                data.base = s;
                while (*s && *s != '}') ++s;
                data.size = s - data.base;
                TEMAKU_DO_LINKS(temaku_writesequence(options, writer, TEMAKU_LINK_START, &data));
                s += !!*s;
                break;
            case 'l':
                TEMAKU_DO_LINKS(temaku_writesequence(options, writer, TEMAKU_LINK_END, NULL));
                break;
            case 'B':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BOLD_START, NULL));
                break;
            case 'b':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BOLD_END, NULL));
                break;
            case 'I':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ITALIC_START, NULL));
                break;
            case 'i':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ITALIC_END, NULL));
                break;
            case 'U':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_UNDERLINE_START, NULL));
                break;
            case 'u':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_UNDERLINE_END, NULL));
                break;
            case 'S':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_STRIKETHROUGH_START, NULL));
                break;
            case 's':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_STRIKETHROUGH_END, NULL));
                break;
            case 'R':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_REVERSE_VIDEO_START, NULL));
                break;
            case 'r':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_REVERSE_VIDEO_END, NULL));
                break;
            case 'A':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ALTERNATIVE_START, NULL));
                break;
            case 'a':
                TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ALTERNATIVE_END, NULL));
                break;
            case 'E':
                TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_BGLINE_START, &bgcolor));
                ctx |= CTX_BGLINE;
                break;
            default:
                in_word = temaku_wordchar(options, seq);
                data.base = &c;
                data.size = 1;
                temaku_writesequence(options, writer, TEMAKU_DATA, &data);
                ++column;
                break;
            }
            break;
default:
put:
            in_word = temaku_wordchar(options, seq);
            data.base = &c;
            data.size = 1;
            temaku_writesequence(options, writer, TEMAKU_DATA, &data);
            ++column;
            break;
        }
    }
    if (ctx & CTX_HEADER)
        TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_HEADER_END, NULL));
    if (ctx & CTX_BOLD)
        TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_BOLD_END, NULL));
    if (ctx & CTX_ITALIC)
        TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_ITALIC_END, NULL));
    if (ctx & CTX_UNDERLINE)
        TEMAKU_DO_STYLE(temaku_writesequence(options, writer, TEMAKU_UNDERLINE_END, NULL));
    if (ctx & CTX_ALTERNATIVE)
        TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_ALTERNATIVE_END, NULL));
    if (ctx & CTX_BGLINE)
        TEMAKU_DO_COLOR(temaku_writesequence(options, writer, TEMAKU_BGLINE_END, NULL));
    data.base = markup;
    data.size = markuplen;
    temaku_writesequence(options, writer, TEMAKU_END, &data);
    return 0;
#undef TEMAKU_DO_LINKS
#undef TEMAKU_DO_STYLE
#undef TEMAKU_DO_COLOR
}
