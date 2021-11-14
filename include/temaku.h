#ifndef TEMAKU_H
#define TEMAKU_H

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/**
 * Macro for defining temaku API symbol declaration
 */
#define TEMAKU_API(T) extern T
/**
 * Macro for defining temaku API function definitions
 */
#define TEMAKU_FUN(T) /* inline */ T
/**
 * Macro for defining temaku API variable definitions
 */
#define TEMAKU_VAR(T) T

/* Forward declaration */
typedef struct temaku_options temaku_options_t;
typedef struct temaku_string temaku_string_t;

/**
 * A string with precalculated length.
 * :type:`temaku_sequence_writer_t` may receive a pointer to this as its argument.
 *
 * @{base}      Pointer to the start of the string.
 * @{size}      Length of the string in bytes.
 */
struct temaku_string {
    const char *base;
    size_t size;
};
/**
 * The type of a sequence to output.
 *
 * @{TEMAKU_START}                  Start a piece of marked-up temaku text.
 *                                  Argument is a :type:`temaku_string_t` containing the text to be marked up.
 * @{TEMAKU_END}                    End a piece of marked-up temaku text.
 *                                  Argument is a :type:`temaku_string_t` containing the text that has been marked up.
 * @{TEMAKU_DATA}                   Write a raw piece of text.
 *                                  Certain character sequences may be escaped here.
 *                                  Argument is a :type:`temaku_string_t` containing the data to write.
 * @{TEMAKU_HEADER_START}           Start a header.
 * @{TEMAKU_HEADER_END}             End a header.
 * @{TEMAKU_BOLD_START}             Start bolding text.
 * @{TEMAKU_BOLD_END}               Stop bolding text.
 * @{TEMAKU_ITALIC_START}           Start italicizing text.
 * @{TEMAKU_ITALIC_END}             Stop italicizing text.
 * @{TEMAKU_UNDERLINE_START}        Start underlining text.
 * @{TEMAKU_UNDERLINE_END}          Stop underlining text.
 * @{TEMAKU_STRIKETHROUGH_START}    Start striking through text.
 * @{TEMAKU_STRIKETHROUGH_END}      Stop striking through text.
 * @{TEMAKU_REVERSE_VIDEO_START}    Start reversing video.
 * @{TEMAKU_REVERSE_VIDEO_END}      Stop reversing video.
 * @{TEMAKU_ALTERNATIVE_START}      Start alternative text.
 * @{TEMAKU_ALTERNATIVE_END}        End alternative text.
 * @{TEMAKU_FGCOLOR_START}          Start coloring text.
 *                                  Argument is a pointer to an ``int`` with the ANSI color, or ``-1`` for no color.
 * @{TEMAKU_FGCOLOR_END}            Stop coloring text.
 *                                  Argument is a pointer to an ``int`` with the ANSI color, or ``-1`` for no color.
 * @{TEMAKU_BGCOLOR_START}          Start coloring background.
 *                                  Argument is a pointer to an ``int`` with the ANSI color, or ``-1`` for no color.
 * @{TEMAKU_BGCOLOR_END}            Stop coloring background.
 *                                  Argument is a pointer to an ``int`` with the ANSI color, or ``-1`` for no color.
 * @{TEMAKU_BGLINE_START}           Fill the rest of the line with the current background color.
 *                                  Argument is a pointer to an ``int`` with the ANSI color, or ``-1`` for no color.
 * @{TEMAKU_BGLINE_END}             End of the line to fill with the current background color.
 * @{TEMAKU_LINK_START}             Start an url link.
 *                                  Argument is a :type:`temaku_string_t` containing the (unescaped) url.
 * @{TEMAKU_LINK_END}               End an url link.
 */
enum temaku_sequence {
    TEMAKU_START,               // /^/
    TEMAKU_END,                 // /$/
    TEMAKU_DATA,                // /./
    TEMAKU_HEADER_START,        // /^=/
    TEMAKU_HEADER_END,          // /$/
    TEMAKU_BOLD_START,          // %B, /(?<!{{wordchar}})*(?={{wordchar}})/
    TEMAKU_BOLD_END,            // %b, /(?<={{wordchar}})*(?!{{wordchar}})/
    TEMAKU_ITALIC_START,        // %I, /(?<!{{wordchar}})\/(?={{wordchar}})/
    TEMAKU_ITALIC_END,          // %i, /(?<={{wordchar}})\/(?!{{wordchar}})/
    TEMAKU_UNDERLINE_START,     // %U, /(?<!{{wordchar}})_(?={{wordchar}})/
    TEMAKU_UNDERLINE_END,       // %u, /(?<={{wordchar}})_(?!{{wordchar}})/
    TEMAKU_STRIKETHROUGH_START, // %S
    TEMAKU_STRIKETHROUGH_END,   // %s
    TEMAKU_REVERSE_VIDEO_START, // %R
    TEMAKU_REVERSE_VIDEO_END,   // %r
    TEMAKU_ALTERNATIVE_START,   // %A, /(?<!{{wordchar}})\|(?!{{wordchar}})/
    TEMAKU_ALTERNATIVE_END,     // %a, /(?<={{wordchar}})\|(?!{{wordchar}})/
    TEMAKU_FGCOLOR_START,       // %F{red}
    TEMAKU_FGCOLOR_END,         // %f
    TEMAKU_BGCOLOR_START,       // %K{red}
    TEMAKU_BGCOLOR_END,         // %k
    TEMAKU_BGLINE_START,        // %E
    TEMAKU_BGLINE_END,          // /\n/
    TEMAKU_LINK_START,          // %L{www.example.com}
    TEMAKU_LINK_END,            // %l
};

/**
 * Generic pointer to the function being called.
 * This function pointer is not callable and should be converted to the type
 * of the function currently being called.
 *
 * Callbacks typically take a pointer to this type as their first argument.
 * This allows callbacks which require contextual data ("userdata") to add it
 * after the function pointer, without requiring extra registers or stack pushes
 * for functions that don't need this.
 *
 * .. code-block:: c
 *
 *    struct callback {
 *        temaku_sequence_writer_t *cb;
 *        void                     *ud;
 *    };
 *    int my_seqwriter(TEMAKU_SELF *self, ...) {
 *        struct callback *cb = (struct callback *)self;
 *        // ...
 *    }
 *    struct callback *writer = malloc(sizeof(struct callback));
 *    writer->cb = my_seqwriter;
 *    writer->ud = (void *)0xdeadbeef;
 *    options.sequence_writer = &writer->cb;
 *    // ... use ``options.sequence_writer``
 *    free(writer);
 *
 */
typedef int (*TEMAKU_SELF)(void);

/**
 * Writer to output data to.
 *
 * @{self}      The pointer to the function pointer currently being called.
 *              See :type:`TEMAKU_SELF` for details.
 * @{data}      The pointer to the data to write.
 * @{size}      The size of the data to write.
 */
typedef int (*temaku_writer_t)(TEMAKU_SELF *self, const void *data, size_t size);
/**
 * Callback to generate sequences with.
 *
 * @{self}      The pointer to the function pointer currently being called.
 *              See :type:`TEMAKU_SELF` for details.
 * @{options}   Options to use for the current invocation.
 * @{writer}    Writer to write the sequence to.
 * @{seq}       The sequence to write.
 * @{arg}       Argument passed by temaku.
 *              Usually a pointer to an integer or a `:type:temaku_string_t`.
 *              See `:type:enum temaku_sequence` for details.
 */
typedef int (*temaku_sequence_writer_t)(TEMAKU_SELF *self, temaku_options_t *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg);

/**
 * Write @{size} bytes from @{data} to writer @{self}.
 */
TEMAKU_API(int) temaku_write(temaku_writer_t *self, const void *data, size_t size);
/**
 * Write the NUL-terminated string @{str} to writer @{self}.
 */
TEMAKU_API(int) temaku_writestr(temaku_writer_t *self, const char *str);
/**
 * Write the ASCII character @{c} to writer @{self}.
 */
TEMAKU_API(int) temaku_writechar(temaku_writer_t *self, char c);
/**
 * Write the integer @{val} in decimal to writer @{self}.
 */
TEMAKU_API(int) temaku_writeint(temaku_writer_t *self, int val);
/**
 * Write the integer @{val} in hexadecimal to writer @{self}.
 */
TEMAKU_API(int) temaku_writehex(temaku_writer_t *self, int val);
/**
 * Write the @{size} bytes in @{url} to writer @{self}.
 * The written string will be URI-escaped.
 */
TEMAKU_API(int) temaku_writeurl(temaku_writer_t *self, const char *url, size_t size);

/**
 * Options for changing the behaviour of temaku.
 *
 * @{sequence_writer}    The sequence writer to invoke when starting sequences.
 * @{wordchars}          Characters that make valid "words".
 *                       The behaviour of certain markup characters (e.g. ``_``)
 *                       does not apply in the middle of a "word".
 * @{string_terminator}  The character to terminate strings with.
 *                       ANSI specifies a "string terminator" sequence which
 *                       is defined in ``TEMAKU_ST``, however, many terminals
 *                       only support terminating strings with the
 *                       ``\x07`` BEL character.
 * @{do_markup}          Set to false to disable all markup.
 * @{do_color}           Set to false to disable coloring.
 * @{do_style}           Set to false to disable text styling (e.g. italic).
 * @{do_links}           Set to false to disable links.
 */
struct temaku_options {
    temaku_sequence_writer_t *sequence_writer;
    const char *wordchars;
    const char *string_terminator;
    bool do_markup;
    bool do_color;
    bool do_style;
    bool do_links;
};

/**
 * BEL string terminator.
 */
#define TEMAKU_BEL "\x07"
/**
 * ANSI string terminator escape sequence.
 */
#define TEMAKU_ST "\x1b\\"

/**
 * Default word characters.
 */
#define TEMAKU_DEFAULT_WORDCHARS "%@!#+-./~_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
/**
 * Default :type:`temaku_options_t` initializer.
 */
#define TEMAKU_DEFAULT_OPTIONS { &temaku_write_ansi_sequence, TEMAKU_DEFAULT_WORDCHARS, TEMAKU_BEL, true, true, true, true }

/**
 * :type:`temaku_options_t` fallback to use when left undefined.
 */
TEMAKU_API(temaku_options_t) temaku_default_options;
/**
 * Sequence writer for ANSI terminal output.
 */
TEMAKU_API(temaku_sequence_writer_t) temaku_write_ansi_sequence;
/**
 * Sequence writer for HTML output.
 */
TEMAKU_API(temaku_sequence_writer_t) temaku_write_html_sequence;

/**
 * Write the sequence @{seq} to the writer @{writer}, using the :type:`temaku_sequence_writer_t` specified in @{options}.
 *
 * @{options}       :type:`temaku_options_t` struct to find sequence writer in and pass along.
 * @{writer}        The writer to write the sequence to.
 * @{seq}           The sequence to write.
 * @{arg}           Argument to pass to the sequence writer.
 *                  Usually a pointer to an integer or a `:type:temaku_string_t`.
 *                  See `:type:enum temaku_sequence` for details.
 */
TEMAKU_API(int) temaku_writesequence(temaku_options_t *options, temaku_writer_t *writer, enum temaku_sequence seq, void *arg);
/**
 * Write the marked-up result of the @{markuplen} bytes in @{markup} to writer @{writer}, using the options specified in @{options}.
 *
 * @{options}       The options to use when writing markup sequences.
 *                  If set to ``NULL``, :var:`temaku_default_options` is used instead.
 * @{writer}        The writer to write the marked-up result to.
 * @{markup}        The temaku markup string to process.
 * @{markuplen}     The length of the markup string.
 *                  If set to ``SIZE_MAX`` or ``0``, processes up to the first NUL character.
 */
TEMAKU_API(int) temaku_markup(temaku_options_t *options, temaku_writer_t *writer, const char *markup, size_t markuplen);

#endif /* TEMAKU_H */
