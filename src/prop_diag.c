/**
 * Property parsing code inspired from g3gg0's Property Editor and Indy's parse_prop.py,
 * but rewritten from scratch :)
 * 
 * This tool can also be built as a standalone program, from the "src" directory:
 *    gcc prop_diag.c -o prop_diag
 * or:
 *    make prop_diag
 * 
 * Python version available on request.
 * 
 * Data structure: in ROM, properties are organized in blocks, sub-blocks and sub-sub-blocks.
 * 
 * Each nesting level uses the same structure:
 * [status_1 size_1 data_1    status_2 size_2 data_2    ...    status_n size_n data_n    terminator]
 * 
 * - "size" covers the size of "status", "size" and "data" fields, so "data" has size-8 bytes
 * - "data" is either an array of sub-blocks [subblock_1 subblock_2 ... subblock_n terminator]
 *    or the property data itself.
 * - "status", "size" and "terminator" are 32-bit integers each.
 * 
 * Consistency check: sum of size_i == buffer_len - 4.
 * There are 4 nesting levels: let's call them "class", "group", "subgroup" and "property".
 * 
 * There are multiple copies of various setting blocks, and only one of them is marked
 * as "active" (status = 0xFFFF); this is probably (just a guess) used for wear leveling
 * and maybe also to save a "last known good" configuration (or that's just a side effect).
 * 
 * New models may have different header structure for the main blocks;
 * in particular, status and size are no longer the first items.
 */

#ifndef CONFIG_MAGICLANTERN
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"
#define bool int
#else
#include "dryos.h"
#endif

#include "property.h"

struct nesting_level
{
    char name[10];
    uint32_t terminator1;
    uint32_t terminator2;
};

struct nesting_level levels[] = {
    { "class",    0x0F000000, 0x1F000000 },
    { "group",    0x00FF0000 },
    { "subgroup", 0x0000FFFF },
    { "property", 0x00000000 },
};

/* camera info extracted from properties */
struct
{
    char* camera_model;
    char* camera_model_alt;
    char* firmware_version;
    char* firmware_subversion;
    char* artist;
    char* copyright;
    char* picstyle_1_name;
    char* picstyle_2_name;
    char* picstyle_3_name;
    char* file_prefix;
    char* dir_suffix;
    int file_number;
} cam_info = {
    "???", "???", "???", "???", "???", "???",
    "???", "???", "???", "????", "?????", 0
};

union prop_data
{
    char str[400];
    int32_t i32[100];
    int8_t i8[400]; 
};

static void process_property(uint32_t property, union prop_data * data, uint32_t data_len)
{
    switch (property)
    {
        case PROP_CAM_MODEL:
            cam_info.camera_model = data->str;
            break;

        case 0x1000010:
            cam_info.camera_model_alt = data->str;
            break;
        
        case PROP_FIRMWARE_VER:
            cam_info.firmware_version = data->str;
            break;
        
        case 0x2000005:
            cam_info.firmware_subversion = data->str;
            break;
        
        case PROP_ARTIST_STRING:
            cam_info.artist = data->str;
            
            /* Canon only deletes the first char when you delete artist name?! */
            if (!cam_info.artist[0] && cam_info.artist[1])
                cam_info.artist++;
            
            break;
        
        case PROP_COPYRIGHT_STRING:
            cam_info.copyright = data->str;

            /* Canon only deletes the first char when you delete copyright string?! */
            if (!cam_info.copyright[0] && cam_info.copyright[1])
                cam_info.copyright++;
            
            break;
        
        case PROP_PC_FLAVOR1_PARAM:
            cam_info.picstyle_1_name = data->str + 4;
            break;

        case PROP_PC_FLAVOR2_PARAM:
            cam_info.picstyle_2_name = data->str + 4;
            break;

        case PROP_PC_FLAVOR3_PARAM:
            cam_info.picstyle_3_name = data->str + 4;
            break;
        
        case PROP_FILE_PREFIX:
        case PROP_USER_FILE_PREFIX:
            cam_info.file_prefix = data->str;
            break;
        
        case PROP_DCIM_DIR_SUFFIX:
            cam_info.dir_suffix = data->str;
            break;
        
        case PROP_FILE_NUMBER_A:
        case PROP_FILE_NUMBER_B:
            if (data->i32[0])
            {
                cam_info.file_number = data->i32[0];
            }
            break;
    }
}

#ifndef CONFIG_MAGICLANTERN
/* Pretty-printer for raw property data */
/* FIXME: standalone only */

/* fixme: move this into a library */
static inline char hex_digit(uint32_t x)
{
    x &= 0xF;
    if (x < 10)
    {
        return '0' + x;
    }

    return 'A' + x - 10;
}

/* fixme: move this into a library */
static int likely_string(const char * str, int min_len)
{
    for (int i = 0; i < min_len; i++)
    {
        /* should match the switch in prop_dump */
        switch (str[i])
        {
            case 32 ... 126:
            case '\n':
            case '\r':
            case '\t':
            case '\b':
                break;
            default:
                return 0;
        }
    }
    return 1;
}

/* fixme: move this into a library */
static int is_empty(const char * str, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (str[i]) return 0;
    }
    return 1;
}

/* Attempts to guess strings, otherwise perform a hex dump.
 * FIXME: refactor without reinventing the wheel. */
const char * prop_dump(const char * data, int len)
{
    static char msg[64];
    int i = 0;
    for (const char * p = data; p < data + len && i < COUNT(msg)-16; p++)
    {
        char c = *p;
        int parsed = p - data;
        int remain = len - parsed;

        switch (c)
        {
            #define NEW_PRINTABLE_CHAR() \
                if (i > 0 && msg[i-1] == '"') { i--; }                      /* append to previous string */ \
                else if (likely_string(p, 4)) { msg[i++] = ' '; msg[i++] = '"'; }  /* start a new string */ \
                else { goto hexdump; }          /* don't start a new string just for 1-2 printable chars */

            case '"':
                NEW_PRINTABLE_CHAR();
                msg[i++] = '\\';
                msg[i++] = '"';
                msg[i++] = '"';
                break;

            case '\0':
                if (is_empty(p, remain)) {
                    /* skip trailing zeros */
                    goto end;
                } else {
                    goto hexdump;
                }
                break;

            case 32 ... '"'-1:
            case '"'+1 ... 126:
                NEW_PRINTABLE_CHAR();
                msg[i++] = c;
                msg[i++] = '"';
                break;
            case '\n':
                NEW_PRINTABLE_CHAR();
                msg[i++] = '\\';
                msg[i++] = 'n';
                msg[i++] = '"';
                break;
            case '\r':
                NEW_PRINTABLE_CHAR();
                msg[i++] = '\\';
                msg[i++] = 'r';
                msg[i++] = '"';
                break;
            case '\t':
                NEW_PRINTABLE_CHAR();
                msg[i++] = '\\';
                msg[i++] = 't';
                msg[i++] = '"';
                break;
            case '\b':
                NEW_PRINTABLE_CHAR();
                msg[i++] = '\\';
                msg[i++] = 'b';
                msg[i++] = '"';
                break;
            default:
            hexdump:
                msg[i++] = ' ';
                if (remain >= 4 && parsed % 4 == 0) {
                    uint32_t val = *(uint32_t *)(p);
                    i += sprintf(msg + i, "%08X", val);

                    int count = 0;
                    for (int k = 0; k < remain / 4; k++)
                    {
                        if (((uint32_t *)p)[k] == val) {
                            count++;
                        } else {
                            break;
                        }
                    }
                    if (count > 1)
                    {
                        i += sprintf(msg + i, "*%d", count);
                    }
                    p += count * 4 - 1;
                } else {
                    msg[i++] = hex_digit(c >> 4);
                    msg[i++] = hex_digit(c);
                }
                break;
        }
        //msg[i] = 0; printf("%s\n", msg);
    }

end:
    if (i >= COUNT(msg) - 16)
    {
        while (i < COUNT(msg) - 4)
            msg[i++] = ' ';
        msg[i++] = '.';
        msg[i++] = '.';
        msg[i++] = '.';
    }

    /* null-terminate */
    while(i >= COUNT(msg));
    msg[i] = 0;

    return msg;
}
#endif

/* parse a property buffer (prop_id, size, data) */
/* buffer_len is always in bytes */
static void parse_property(uint32_t * buffer, uint32_t buffer_len)
{
    uint32_t prop_id = buffer[0];
    uint32_t size = buffer[1] - 8;
    uint32_t * data = &buffer[2];
    // assert len(data) == size
    
    #ifndef CONFIG_MAGICLANTERN
    printf("Prop %8x %6d %s\n", prop_id, size, prop_dump((char*)data, size));
    #endif

    process_property(prop_id, (union prop_data *) data, size);
}

static int check_terminator(int level, uint32_t tail, int verbose)
{
    if (levels[level].terminator1 == 0)
    {
        /* nothing to check */
        return -1;
    }
    
    if (levels[level].terminator1 == tail)
    {
        /* check if terminator matches the current group's tail */
        return 1;
    }

    if (levels[level].terminator2)
    {
        /* some property blocks may use different terminator values */
        if (levels[level].terminator2 == tail)
        {
            return 1;
        }
    }

    if (verbose)
    {
        printf("%s end error (0x%x, expected 0x%x or 0x%x)\n",
            levels[level].name, tail,
            levels[level].terminator1,
            levels[level].terminator2
        );
    }

    return 0;
}

/* parse a class of groups of subgroups of properties (recursive) */
static int parse_prop_group(uint32_t * buffer, int buffer_len, int status_idx, int size_idx, int level, int verbose, int output)
{
    //~ printf("parse prop group %p %x %d\n", buffer, buffer_len, level);

    if (buffer_len % 4)
    {
        if (verbose) printf("%s buffer len align error (0x%x)\n", levels[level].name, buffer_len);
        return 0;
    }

    if (buffer_len == 4)
    {
        /* found in M50/SX70 */
        if (verbose) printf("%s empty?\n", levels[level].name);
        return 1;
    }

    if (buffer_len < size_idx * 4 + 4)
    {
        if (verbose) printf("%s buffer len error (0x%x, expected at least 0x%x)\n", levels[level].name, buffer_len, size_idx * 4 + 4);
        return 0;
    }

    uint32_t status = buffer[status_idx];
    int size = buffer[size_idx];

    if (size == 0 && status == 0)
    {
        /* found in 500D */
        if (verbose) printf("%s empty?\n", levels[level].name);
        return 1;
    }

    if (size < size_idx * 4 + 8 || size > buffer_len)
    {
        if (verbose) printf("%s size error (0x%x, expected between 0x0c and 0x%x)\n", levels[level].name, size, buffer_len);
        return 0;
    }
    
    uint32_t tail = buffer[size/4 - 1];

    if (output && levels[level].terminator1 == 0)
    {
        parse_property(buffer, size);
    }
    
    if (verbose)
    {
        printf("%s: status=0x%x size=0x%x tail=0x%x\n", levels[level].name, status, size, tail);
    }

    if (check_terminator(level, tail, verbose))
    {
        if (level+1 < COUNT(levels))
        {
            int ok = parse_prop_group(buffer + (size_idx+1), size - (size_idx+1)*4, 0, 1, level+1, verbose, output);
            if (!ok)
            {
                if (verbose) printf("%s: check failed\n", levels[level+1].name);
                return 0;
            }
        }
    }
    else
    {
        /* error message was printed in check_terminator */
        return 0;
    }

    if (buffer_len - size == 4)
    {
        if (verbose) printf("%s end OK\n", levels[level].name);
        return 1;
    }
    else
    {
        if (verbose) printf("%s: 0x%x bytes remaining\n", levels[level].name, buffer_len - size);
        return parse_prop_group(buffer + size/4, buffer_len - size, status_idx, size_idx, level, verbose, output);
    }

    return 1;
}

static void guess_offsets(uint32_t * buffer, uint32_t * status_idx, uint32_t * size_idx, uint32_t * active_flag)
{
    /* most models have their property data structures starting with "status" and "size" */
    *status_idx  = 0;
    *size_idx    = 1;
    *active_flag = 0xFFFF;

    if (buffer[0] == 2)
    {
        /* 1300D data structure starts with 2 */
        *status_idx = 3;
        *size_idx   = 4;
    }
    else if (buffer[1] == 0xFFFFFFFF)
    {
        /* M50 has a bunch of fields before size, mostly FFFFFFFF */
        *status_idx  = 4;
        *size_idx    = 12;
        *active_flag = 0xFFFFFFFF;
    }
}

/* Brute force: find the offsets that look like valid property data structures */
static void guess_prop(uint32_t * buffer, uint32_t buffer_len, int active_only, int verbose)
{
    /* these properties must be aligned, right? */
    //~ for (uint32_t offset = 0x8fa000; offset; offset = 0)    /* for testing with some particular offset */
    for (uint32_t offset = 0; offset < buffer_len; offset += 0x100)
    {
        uint32_t status_idx, size_idx, active_flag;
        guess_offsets(&buffer[offset/4], &status_idx, &size_idx, &active_flag);
        uint32_t status = buffer[offset/4 + status_idx];
        uint32_t size = buffer[offset/4 + size_idx];

        if (size > 0 && (size & 3) == 0)                        // size looks 32-bit aligned?
        {
            uint32_t last_pos = offset + size - 4;
            if (last_pos > size_idx * 4 + 8 &&
                last_pos < buffer_len-4)                        // not out of range?
            {
                uint32_t last = buffer[last_pos/4];
                if (check_terminator(0, last, 0))               // terminator OK?
                {
                    if (verbose)
                    {
                        printf("Trying offset 0x%x, status=0x%x, size=0x%x...\n", offset, status, size);
                    }

                    /* let's try to parse it quietly, without any messages */
                    /* if successful, will parse again with output enabled */
                    if (parse_prop_group(buffer + offset/4, size+4, status_idx, size_idx, 0, verbose == 2, 0))
                    {
                        if (active_only && status != active_flag)
                        {
                            /* skip inactive block */
                            if (verbose)
                            {
                                printf("Skipping inactive block 0x%x, status=0x%x, size=0x%x...\n", offset, status, size);
                            }
                            continue;
                        }

                        parse_prop_group(buffer + offset/4, size+4, status_idx, size_idx, 0, verbose == 2, 1);
                    }
                }
            }
        }
    }
}

static void print_camera_info()
{
    if (strcmp(cam_info.camera_model, cam_info.camera_model_alt) == 0 ||
        strcmp(cam_info.camera_model_alt, "???") == 0 ||
        strcmp(cam_info.camera_model_alt, "Canon EOS 60D") == 0)    /* 5D3 quirk */
    {
        /* many models use only this one */
        printf(" - Camera model: %s\n", cam_info.camera_model);
    }
    else if (strcmp(cam_info.camera_model, "???") == 0)
    {
        /* a few models use only this one */
        printf(" - Camera model: %s\n", cam_info.camera_model_alt);
    }
    else
    {
        /* some models have two names, e.g. "Canon EOS 6D Mark II" and "Canon EOS K406" */
        /* any of them might be the code name or the market name, so... let's just print both */
        /* FIXME: result not pretty if one of the names is prefix of the other */
        int common = 0;
        while (toupper(cam_info.camera_model[common]) == toupper(cam_info.camera_model_alt[common])) common++;
        printf(" - Camera model: %s / %s\n", cam_info.camera_model_alt, cam_info.camera_model + common);
    }

    printf(" - Firmware version: %s / %s\n", cam_info.firmware_version, cam_info.firmware_subversion);
    printf(" - IMG naming: 100%s/%s%04d.JPG\n", cam_info.dir_suffix, cam_info.file_prefix, cam_info.file_number);
#if 0
    if (cam_info.artist[0])
    {
        printf(" - Artist: %s\n", cam_info.artist);
    }
    if (cam_info.copyright[0])
    {
        printf(" - Copyright: %s\n", cam_info.copyright);
    }
#endif
    if (cam_info.picstyle_1_name[0] || cam_info.picstyle_2_name[0] || cam_info.picstyle_3_name[0])
    {
        printf(" - User PS: %s %s %s\n", cam_info.picstyle_1_name, cam_info.picstyle_2_name, cam_info.picstyle_3_name);
    }
}

#ifdef CONFIG_MAGICLANTERN
/* for running on the camera */

uint32_t is_digic6();
uint32_t is_digic78();

void prop_diag()
{
    if (is_digic78())
    {
        /* other models may lock up while reading this, so test first */
        guess_prop((void*)0xE0000000, 0x2000000, 1, 0);
        guess_prop((void*)0xF0000000, 0x1000000, 1, 0);
    }
    else if (is_digic6())
    {
        guess_prop((void*)0xFC000000, 0x2000000, 1, 0);
    }
    else
    {
        guess_prop((void*)0xF0000000, 0x2000000, 1, 0);
        guess_prop((void*)0xF8000000, 0x2000000, 1, 0);
    }
    print_camera_info();
}
#else
/* for offline testing */
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: %s ROM0.BIN\n", argv[0]);
        return 1;
    }
    printf("Loading %s...\n", argv[1]);
    char* filename = argv[1];
    FILE* f = fopen(filename, "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        int len = ftell(f);
        fseek(f, 0, SEEK_SET);
        uint32_t * buf = malloc(len);
        fread(buf, len, 1, f);
        fclose(f);
        
        printf("Scanning from %p to %p...\n", buf, ((void*)buf) + len);
        guess_prop(buf, len, 1, 1);
        print_camera_info();
    }
    return 0;
}
#endif