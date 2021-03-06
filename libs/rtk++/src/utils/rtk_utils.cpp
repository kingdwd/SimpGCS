/******************************************************************************

  Robot Toolkit ++ (RTK++)

  Copyright (c) 2007-2013 Shuhui Bu <bushuhui@nwpu.edu.cn>
  http://www.adv-ci.com

  ----------------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include <math.h>
#include <complex.h>
#include <float.h>

#ifdef RTK_LINUX
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#endif

#ifdef RTK_WIN
#include <windows.h>
#endif

#include <errno.h>

#include <assert.h>
#include <inttypes.h>
#include <tmmintrin.h>

#include <string>
#include <vector>
#include <algorithm>

#include "rtk_debug.h"
#include "rtk_utils.h"

using namespace std;

namespace rtk {

////////////////////////////////////////////////////////////////////////////////
/// time functions
////////////////////////////////////////////////////////////////////////////////

ru64 tm_get_millis(void)
{
#ifdef RTK_LINUX
    struct timeval  tm_val;
    ru64            v;
    int             ret;

    ret = gettimeofday(&tm_val, NULL);

    v = tm_val.tv_sec*1000 + tm_val.tv_usec/1000;
    return v;
#endif

#ifdef RTK_WIN
    return GetTickCount();
#endif
}

ru64 tm_get_ms(void)
{
#ifdef RTK_LINUX
    struct timeval  tm_val;
    ru64            v;
    int             ret;

    ret = gettimeofday(&tm_val, NULL);

    v = tm_val.tv_sec*1000 + tm_val.tv_usec/1000;
    return v;
#endif

#ifdef RTK_WIN
    return GetTickCount();
#endif
}

ru64 tm_get_us(void)
{
#ifdef RTK_LINUX
    struct timeval  tm_val;
    ru64            v;
    int             ret;

    ret = gettimeofday(&tm_val, NULL);

    v = tm_val.tv_sec*1000000 + tm_val.tv_usec;
    return v;
#endif

#ifdef RTK_WIN
    FILETIME        t;
    ru64            t_ret;

    // get UTC time
    GetSystemTimeAsFileTime(&t);

    t_ret = 0;

    t_ret |= t.dwHighDateTime;
    t_ret <<= 32;
    t_ret |= t.dwLowDateTime;

    // convert 100 ns to [ms]
    t_ret /= 10;

    return t_ret;
#endif
}

void   tm_sleep(ru32 t)
{
#ifdef RTK_LINUX
    struct timespec tp;

    tp.tv_sec  = t / 1000;
    tp.tv_nsec = ( t % 1000 ) * 1000000;

    nanosleep(&tp, &tp);
#endif

#ifdef RTK_WIN
    Sleep(ms);
#endif
}


////////////////////////////////////////////////////////////////////////////////
/// string functions
////////////////////////////////////////////////////////////////////////////////

// split given string by delims
StringArray split_text_cpp(const string &intext, const string &delims)
{
    StringArray         r;
    string::size_type   begIdx, endIdx;
    string              s;

    begIdx = intext.find_first_not_of(delims);

    while(begIdx != string::npos) {
        // search end of the word
        endIdx = intext.find_first_of(delims, begIdx);
        if( endIdx == string::npos )
            endIdx = intext.length();

        // get the sub string
        s = intext.substr(begIdx, endIdx-begIdx);
        r.push_back(s);

        // find next begin position
        begIdx = intext.find_first_not_of(delims, endIdx);
    }

    return r;
}

inline int char_is_delims(char c, char d)
{
    if( c == d )
        return 1;
    else
        return 0;
}

inline int char_is_newline(char c)
{
    if( c == 0x0d || c == 0x0a )
        return 1;
    else
        return 0;
}

StringArray split_text(const string &intext, const string &delims)
{
    StringArray         r;

    int                 st;
    int                 n, nd, i, j, k, dd, ns;
    char                *buf;
    const char          *pb, *pd;


    n = intext.size();
    nd = delims.size();

    pb = intext.c_str();
    pd = delims.c_str();

    buf = new char[n+10];

    st = 0;
    i = 0;
    k = 0;
    ns = 0;
    buf[0] = 0;

    while( i<n ) {
        for(dd = 0, j=0; j<nd; j++) dd += char_is_delims(pb[i], pd[j]);

        if( dd > 0 ) {
            buf[k] = 0;
            r.push_back(buf);

            k = 0;
            st = 1;
        } else {
            buf[k++] = pb[i];
            st = 0;
        }

        i++;
    }

    // process last character
    if( st == 0 ) {
        buf[k] = 0;
        r.push_back(buf);
    } else {
        buf[0] = 0;
        r.push_back(buf);
    }

    delete [] buf;

    return r;
}

///
/// \brief split_line
/// \param intext - input string
/// \return
///
/// FIXME: blank line is not supported!
///
StringArray split_line(const std::string &intext)
{
    StringArray         r;

    int                 st;
    int                 n, i, k, dd;
    char                *buf;
    const char          *pb;

    n = intext.size();
    pb = intext.c_str();

    buf = new char[n+10];

    st = 0;
    i = 0;
    k = 0;
    buf[0] = 0;

    while( i<n ) {
        dd = char_is_newline(pb[i]);

        if( dd > 0 ) {
            if( k > 0 ) {
                buf[k] = 0;
                r.push_back(buf);

                k = 0;
                st = 1;
            }
        } else {
            buf[k++] = pb[i];
            st = 0;
        }

        i++;
    }

    // process last character
    if( st == 0 ) {
        buf[k] = 0;
        r.push_back(buf);
    }

    delete [] buf;

    return r;
}

// string trim functions
string ltrim(const string &s)
{
    string              delims = " \t\n\r",
                        r;
    string::size_type   i;

    i = s.find_first_not_of(delims);
    if( i == string::npos )
        r = "";
    else
        r = s.substr(i, s.size() - i);

    return r;
}


string rtrim(const string &s)
{
    string              delims = " \t\n\r",
                        r;
    string::size_type   i;

    i = s.find_last_not_of(delims);
    if( i == string::npos )
        r = "";
    else
        r = s.substr(0, i+1);

    return r;
}


string trim(const string &s)
{
    string              delims = " \t\n\r",
                        r;
    string::size_type   i, j;

    i = s.find_first_not_of(delims);
    j = s.find_last_not_of(delims);

    if( i == string::npos ) {
        r = "";
        return r;
    }

    if( j == string::npos ) {
        r = "";
        return r;
    }

    r = s.substr(i, j-i+1);
    return r;
}

string str_tolower(string &s)
{
    for(int i=0; i < s.size(); i++) {
        s[i] = tolower(s[i]);
    }

    return s;
}

string str_toupper(string &s)
{
    for(int i=0; i < s.size(); i++) {
        s[i] = toupper(s[i]);
    }

    return s;
}


int str_to_int(const string &s)
{
    return atoi(s.c_str());
}

float str_to_float(const string &s)
{
    return atof(s.c_str());
}

double str_to_double(const string &s)
{
    return atof(s.c_str());
}


////////////////////////////////////////////////////////////////////////////////
/// arguments functions
////////////////////////////////////////////////////////////////////////////////

void save_arguments(int argc, char *argv[], string &fname)
{
    string      fn;
    FILE        *fp;
    int         i;
    tm          *now;
    time_t      t;
    char        str_time[200];


    fn = fname + "_args.txt";
    fp = fopen(fn.c_str(), "a+"); ASSERT(fp);

    // get current time
    time(&t);
    now = localtime(&t);
    strftime(str_time, 200, "%Y-%m-%d %H:%M:%S", now);

    fprintf(fp, "--------------- %s ---------------\n", str_time);

    for(i=0; i<argc; i++)
        fprintf(fp, "%s ", argv[i]);

    fprintf(fp, "\n\n");

    fclose(fp);
}


////////////////////////////////////////////////////////////////////////////////
/// file & path functions
////////////////////////////////////////////////////////////////////////////////

long filelength(FILE *fp)
{
    long    len;

    if( fp == NULL )
        return 0;

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return len;
}

long filelength(const char *fname)
{
    FILE    *fp;
    long    len;

    fp = fopen(fname, "r");
    if( fp == NULL )
        return 0;

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fclose(fp);

    return len;
}

int path_exist(const char *p)
{
    struct stat     st;
    int             ret;

    ret = stat(p, &st);
    return ret;
}

int path_mkdir(const char *p)
{
    char            cmd[2048];
    int             ret;

    // FIXME: only support UNIX system
    //ret = mknod(p, S_IFDIR | 0775, 0);
    sprintf(cmd, "mkdir -p '%s'", p);

    ret = system(cmd);
    if( ret != 0 ) ret = -1;

    return ret;
}

int path_delfile(const std::string &p)
{
    char            cmd[2048];
    int             ret;

    // FIXME: only support UNIX system
    sprintf(cmd, "rm -rf '%s'", p.c_str());

    ret = system(cmd);
    if( ret != 0 ) ret = -1;

    return ret;
}


int path_lsdir(const string &dir_name, StringArray &dl)
{
    DIR             *dir;
    struct dirent   *dp;

    // open directory
    dir = opendir(dir_name.c_str());
    if( dir == NULL ) {
        dbg_pe("Failed to open dir: %s\n", dir_name.c_str());
        return -1;
    }

    // get each items
    dl.clear();
    for(dp=readdir(dir); dp!=NULL; dp=readdir(dir)) {
        // skip .
        if( strlen(dp->d_name) == 1 && dp->d_name[0] == '.' )
            continue;

        // skip ..
        if( strlen(dp->d_name) == 2 && dp->d_name[0] == '.' && dp->d_name[1] == '.' )
            continue;

        // add to list
        dl.push_back(dp->d_name);
    }

    closedir(dir);

    // sort all file name
    std::sort(dl.begin(), dl.end());
}

int path_isdir(const std::string &p)
{
    struct stat     st;
    int             ret;

    ret = stat(p.c_str(), &st);
    if( ret == -1 ) {
        dbg_pe("Failed at stat! (%s)", p.c_str());
        return 0;
    }

    if ( (st.st_mode & S_IFMT) == S_IFDIR )
        return 1;
    else
        return 0;
}

int path_isfile(const std::string &p)
{
    struct stat     st;
    int             ret;

    ret = stat(p.c_str(), &st);
    if( ret == -1 ) {
        dbg_pe("Failed at stat! (%s)", p.c_str());
        return 0;
    }

    if ( (st.st_mode & S_IFMT) == S_IFREG )
        return 1;
    else
        return 0;
}

const std::string auto_filename(const char *fn_base)
{
    RDateTime    t;

    t.setCurrentDateTime();

    if( fn_base == NULL ) {
        return fmt::sprintf("autosave_%04d%02d%02d_%02d%02d%02d",
                            t.year, t.month, t.day,
                            t.hour, t.min, t.sec);
    } else{
        return fmt::sprintf("%s_%04d%02d%02d_%02d%02d%02d",
                            fn_base,
                            t.year, t.month, t.day,
                            t.hour, t.min, t.sec);
    }
}

const std::string auto_filename(const std::string &fn_base)
{
    RDateTime    t;

    t.setCurrentDateTime();

    if( fn_base.size() == 0 ) {
        return fmt::sprintf("autosave_%04d%02d%02d_%02d%02d%02d",
                            t.year, t.month, t.day,
                            t.hour, t.min, t.sec);
    } else{
        return fmt::sprintf("%s_%04d%02d%02d_%02d%02d%02d",
                            fn_base,
                            t.year, t.month, t.day,
                            t.hour, t.min, t.sec);
    }
}



StringArray path_split(const string &fname)
{
    size_t      found = -1;
    StringArray r;

    r.clear();

    /* find / or \ */
    found = fname.find_last_of("/\\");

    if( found == string::npos ) {
        r.push_back("");
        r.push_back(fname);
        return r;
    }

    // folder
    r.push_back(fname.substr(0, found));
    // file
    r.push_back(fname.substr(found+1));

    return r;
}

StringArray path_splitext(const string &fname)
{
    size_t      found;
    StringArray r;

    r.clear();

    // find .
    found = fname.find_last_of(".");
    if( found == string::npos ) {
        r.push_back(fname);
        r.push_back("");
        return r;
    }

    // filename
    r.push_back(fname.substr(0, found));
    // extname
    r.push_back(fname.substr(found));

    return r;
}

std::string path_extname(std::string &fname)
{
    size_t      found;
    string      ext_name = "";


    // find .
    found = fname.find_last_of(".");
    if( found != string::npos ) {
        ext_name = fname.substr(found);
    }

    return ext_name;
}

std::string path_join(const std::string &p1, const std::string &p2)
{
    string      p;
    int         l;

    p = p1;

    l = p.size();
    if( p[l-1] == '/' || p[l-1] == '\\' )
        p = p.substr(0, l-1);

    p = p + "/" + p2;
    return p;
}

std::string path_join(const std::string &p1, const std::string &p2, const std::string &p3)
{
    string      p;

    p = path_join(p1, p2);
    return path_join(p, p3);
}


std::string path_join(const StringArray &p)
{
    int     i, l;
    string  p_all;

    p_all = "";
    for(i=0; i<p.size(); i++) {
        l = p_all.size();
        if( l>0 ) {
            if( p_all[l-1] == '/' || p_all[l-1] == '\\' )
                p_all = p_all.substr(0, l-1);
        }

        p_all = p_all + "/" + p[i];
    }

    return p_all;
}


////////////////////////////////////////////////////////////////////////////////
/// text file functions
////////////////////////////////////////////////////////////////////////////////

int readlines(const char *fn, StringArray &lns, int buf_len)
{
    FILE    *fp=NULL;

    char    *buf;
    string  s;

    // clear old data
    lns.clear();

    // alloc buffer
    buf = new char[buf_len];

    // open file
    fp = fopen(fn, "r");
    if( fp == NULL ) {
        ASSERT(fp);
        return -1;
    }

    while( !feof(fp) ) {
        // read a line
        if( NULL == fgets(buf, buf_len, fp) )
            break;

        // remove blank & CR
        s = trim(buf);

        // skip blank line
        if( s.size() < 1 )
            continue;

        // add to list
        lns.push_back(s);
    }

    // close file
    fclose(fp);

    // free array
    delete [] buf;

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
/// array write/read functions
////////////////////////////////////////////////////////////////////////////////

int save_darray(const char *fn, ru64 n, double *d)
{
    FILE        *fp;

    fp = fopen(fn, "w"); ASSERT(fp);

    fwrite(&n, sizeof(ru64),    1, fp);
    fwrite(d,  sizeof(double)*n,    1, fp);
    fclose(fp);

    return 0;
}

int load_darray(const char *fn, ru64 &n, double **d)
{
    FILE        *fp;
    double      *d_;
    int         i;

    fp = fopen(fn, "r"); ASSERT(fp);

    i = fread(&n, sizeof(ru64), 1, fp);

    d_ = new double[n];
    i = fread(d_, sizeof(double)*n, 1, fp);
    fclose(fp);

    *d = d_;

    return 0;
}

int save_farray(const char *fn, ru64 n, float *d)
{
    FILE        *fp;

    fp = fopen(fn, "w"); ASSERT(fp);

    fwrite(&n, sizeof(ru64),    1, fp);
    fwrite(d,  sizeof(float)*n,     1, fp);
    fclose(fp);

    return 0;
}

int load_farray(const char *fn, ru64 &n, float **d)
{
    FILE        *fp;
    float       *d_;
    int         i;

    fp = fopen(fn, "r"); ASSERT(fp);

    i = fread(&n, sizeof(ru64), 1, fp);

    d_ = new float[n];
    i = fread(d_, sizeof(float)*n,  1, fp);
    fclose(fp);

    *d = d_;

    return 0;
}



int save_darray(const char *fn, ru64 n, ru64 m, double *d)
{
    FILE        *fp;

    fp = fopen(fn, "w"); ASSERT(fp);

    fwrite(&n, sizeof(ru64),    1, fp);
    fwrite(&m, sizeof(ru64),    1, fp);
    fwrite(d,  sizeof(double)*n*m,  1, fp);
    fclose(fp);

    return 0;
}

int load_darray(const char *fn, ru64 &n, ru64 &m, double **d)
{
    FILE        *fp;
    double      *d_;
    int         i;

    fp = fopen(fn, "r"); ASSERT(fp);

    i = fread(&n, sizeof(ru64), 1, fp);
    i = fread(&m, sizeof(ru64), 1, fp);

    d_ = new double[n*m];
    i = fread(d_, sizeof(double)*n*m, 1, fp);
    fclose(fp);

    *d = d_;

    return 0;
}

int save_farray(const char *fn, ru64 n, ru64 m, float *d)
{
    FILE        *fp;

    fp = fopen(fn, "w"); ASSERT(fp);

    fwrite(&n, sizeof(ru64),    1, fp);
    fwrite(&m, sizeof(ru64),    1, fp);
    fwrite(d,  sizeof(float)*n*m,   1, fp);
    fclose(fp);

    return 0;
}

int load_farray(const char *fn, ru64 &n, ru64 &m, float **d)
{
    FILE        *fp;
    float       *d_;
    int         i;

    fp = fopen(fn, "r"); ASSERT(fp);

    i = fread(&n, sizeof(ru64), 1, fp);
    i = fread(&m, sizeof(ru64), 1, fp);

    d_ = new float[n*m];
    i = fread(d_, sizeof(float)*n*m,  1, fp);
    fclose(fp);

    *d = d_;

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
/// memory function
////////////////////////////////////////////////////////////////////////////////
void memcpy_fast(void *dst, void *src, ru32 s)
{
    ru8    *_src, *_dst;
    ru8    *end;
    ru32   _s;

    _src = (ru8*) src;
    _dst = (ru8*) dst;

    // check memory address is aligned
    assert((uintptr_t)_src % 16 == 0);

    // first part using SSE
    _s = (s/16)*16;
    end = _src + _s;
    for (; _src != end; _src += 16, _dst += 16) {
        _mm_storeu_si128((__m128i *) _dst, _mm_load_si128((__m128i *) _src));
    }

    // remainning part using normal copy
    end = (ru8*)src + s;
    for(; _src != end; _src++, _dst++) {
        *_dst = *_src;
    }
}

void memcpy_fast_(void *dst, void *src, ru32 s)
{
    ru8    *p_src, *p_dst;
    ru32   n_fast, n_last;

    __m128i *src_vec = (__m128i*) src;
    __m128i *dst_vec = (__m128i*) dst;

    // check memory address is aligned
    assert((uintptr_t)src % 16 == 0);

    n_fast = s/64;
    n_last = s - n_fast*64;

    p_src = (ru8*) src;
    p_dst = (ru8*) dst;
    p_src += n_fast*64;
    p_dst += n_fast*64;

    // first part using SSE
    while( n_fast-- > 0 ) {
        _mm_storeu_si128(dst_vec,   _mm_load_si128(src_vec));
        _mm_storeu_si128(dst_vec+1, _mm_load_si128(src_vec+1));
        _mm_storeu_si128(dst_vec+2, _mm_load_si128(src_vec+2));
        _mm_storeu_si128(dst_vec+3, _mm_load_si128(src_vec+3));

        dst_vec += 4;
        src_vec += 4;
    }

    // remainning part using normal copy
    while( n_last-- > 0 ) {
        *p_dst = *p_src;
        p_src ++;
        p_dst ++;
    }
}



void conv_argb8888_bgr888(ru8 *src, ru8 *dst, ru32 s)
{
    ru32 i;

    for(i=0; i<s; i++) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];

        src += 4;
        dst += 3;
    }
}

// needs:
// src is 16-byte aligned
void conv_argb8888_bgr888_fast(ru8 *src, ru8 *dst, ru32 s)
{
    ru32   _s1, _s2;
    ru32   i;
    ru8    *end;

    // check memory address is aligned
    assert((uintptr_t)src % 16 == 0);

    _s1 = (s/4-1)*4;
    _s2 = s - _s1;

    // major part using SSE copy
    __m128i mask = _mm_set_epi8(
                -128, -128, -128, -128,
                12, 13, 14,
                8,  9, 10,
                4,  5,  6,
                0,  1,  2);
    end = src + _s1 * 4;
    for (; src != end; src += 16, dst += 12) {
        _mm_storeu_si128((__m128i *) dst, _mm_shuffle_epi8(_mm_load_si128((__m128i *) src), mask));
    }

    // remainning part using normal copy
    for (i=0; i<_s2; i++) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];

        src += 4;
        dst += 3;
    }
}


void conv_bgr888_argb8888(ru8 *src, ru8 *dst, ru32 s)
{
    ru32 i;

    for(i=0; i<s; i++) {
        dst[0] = 255;
        dst[1] = src[2];
        dst[2] = src[1];
        dst[3] = src[0];

        src += 3;
        dst += 4;
    }
}


void conv_bgr888_argb8888_fast(ru8 *src, ru8 *dst, ru32 s)
{
    __m128i *in_vec  = (__m128i*) src;
    __m128i *out_vec = (__m128i*) dst;

    __m128i in1, in2, in3;
    __m128i out;

    __m128i x0 = _mm_set_epi8(0,  0,  0, 0xff, 0, 0, 0, 0xff, 0, 0, 0, 0xff, 0, 0, 0, 0xff);
    __m128i x1 = _mm_set_epi8(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0);
    __m128i x2 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);

    __m128i m1 = _mm_set_epi8(9,  10, 11, 0xff,  6,  7,  8, 0xff,  3,  4,  5, 0xff,  0,  1,  2, 0xff);
    __m128i m2 = _mm_set_epi8(5,   6,  7, 0xff,  2,  3,  4, 0xff, 15,  0,  1, 0xff, 12, 13, 14, 0xff);
    __m128i m3 = _mm_set_epi8(1,   2,  3, 0xff, 14, 15,  0, 0xff, 11, 12, 13, 0xff,  8,  9, 10, 0xff);
    __m128i m4 = _mm_set_epi8(13, 14, 15, 0xff, 10, 11, 12, 0xff,  7,  8,  9, 0xff,  4,  5,  6, 0xff);


    ri32 n_fast, n_last;
    ru8 *p_in, *p_out;

    // check memory address is aligned
    assert((uintptr_t)src % 16 == 0);

    n_fast = s / 16;
    n_last = s - n_fast*16;

    p_in  = src + n_fast*16*3;
    p_out = dst + n_fast*16*4;


    // fast conv
    while (n_fast-- > 0) {
        // conver first 4 pixel
        in1 = in_vec[0];
        out = _mm_shuffle_epi8(in1, m1);
        out = _mm_or_si128(out, x0);
        out_vec[0] = out;

        // convert second 4 pixel
        in2 = in_vec[1];
        in1 = _mm_and_si128(in1, x1);
        out = _mm_and_si128(in2, x2);
        out = _mm_or_si128(out, in1);
        out = _mm_shuffle_epi8(out, m2);
        out = _mm_or_si128(out, x0);
        out_vec[1] = out;

        // conver third 4 pixel
        in3 = in_vec[2];
        in2 = _mm_and_si128(in2, x1);
        out = _mm_and_si128(in3, x2);
        out = _mm_or_si128(out, in2);
        out = _mm_shuffle_epi8(out, m3);
        out = _mm_or_si128(out, x0);
        out_vec[2] = out;

        // convert forth 4 pixel
        out = _mm_shuffle_epi8(in3, m4);
        out = _mm_or_si128(out, x0);
        out_vec[3] = out;

        // point to next 16 pixel
        in_vec  += 3;
        out_vec += 4;
    }

    while( n_last-- > 0 ) {
        p_out[0] = 255;
        p_out[1] = p_in[2];
        p_out[2] = p_in[1];
        p_out[3] = p_in[0];
    }
}


void conv_rgb888_bgr888(ru8 *src, ru8 *dst, ru32 s)
{
    ru32 i;
    for(i=0; i<s; i++) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];

        src += 3;
        dst += 3;
    }
}

/* in and out must be 16-byte aligned */
void conv_rgb888_bgr888_fast(ru8 *in, ru8 *out, ru32 s)
{
    __m128i *in_vec  = (__m128i*) in;
    __m128i *out_vec = (__m128i*) out;

    __m128i m1 = _mm_set_epi8(-128, 12, 13, 14, 9, 10, 11, 6, 7, 8, 3, 4, 5, 0, 1,  2);
    __m128i m2 = _mm_set_epi8(15, -128, 11, 12, 13, 8, 9, 10, 5, 6, 7, 2, 3, 4, -128, 0);
    __m128i m3 = _mm_set_epi8(13, 14, 15, 10, 11, 12, 7, 8, 9, 4, 5, 6, 1, 2, 3, -128);

    ru32 n_fast, n_last;
    ru8  *p_in, *p_out;

    // check memory address is aligned
    assert((uintptr_t)in % 16 == 0);

    n_fast = s / 48;
    n_last = s - n_fast*48;

    p_in = in;
    p_out = out;

    // fast copy
    while (n_fast-- > 0) {
        _mm_storeu_si128(out_vec,   _mm_shuffle_epi8(_mm_load_si128(in_vec),   m1));
        _mm_storeu_si128(out_vec+1, _mm_shuffle_epi8(_mm_load_si128(in_vec+1), m2));
        _mm_storeu_si128(out_vec+2, _mm_shuffle_epi8(_mm_load_si128(in_vec+2), m3));

        p_out[15] = p_in[17];
        p_out[17] = p_in[15];
        p_out[30] = p_in[32];
        p_out[32] = p_in[30];

        in_vec  += 3;
        out_vec += 3;
        p_in    += 48;
        p_out   += 48;
    }

    // remaining items using normal copy
    while(n_last-- > 0 ) {
        p_out[0] = p_in[2];
        p_out[1] = p_in[1];
        p_out[2] = p_in[0];

        p_in  += 3;
        p_out += 3;
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Color pallete
////////////////////////////////////////////////////////////////////////////////

/**
 *  カラー定義
 *      0. グレースケール
 *      1. 赤→薄緑→青
 *      2. 赤→緑→青
 *      3. Fire1(黒→赤→橙→黄→白)
 *      4. Fire2(黒→濃紫→橙→黄→白)
 *      5. Fresh(青→水色→薄黄緑→黄→赤)
 *      6. Hitachi(青→水色→黄緑→黄→赤)
 *      7. Doppler(青→黒→赤)
 *      8. Jet(Blue -> Red)
 */

unsigned char Cr[]={
/*0.*/0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
/*1.*/0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
/*2.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,255,
/*3.*/0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*4.*/0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*5.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,65,67,68,70,71,73,74,76,77,79,80,82,83,85,86,88,89,91,92,94,95,97,98,100,101,103,104,106,107,109,110,112,113,115,116,118,119,121,122,124,125,127,128,130,131,133,134,136,137,139,140,142,143,145,146,148,149,151,152,154,155,157,158,160,161,163,164,166,167,169,170,172,173,175,176,178,179,181,182,184,185,187,188,190,191,193,194,196,197,199,200,202,203,205,206,208,209,211,212,214,215,217,218,220,221,223,224,226,227,229,230,232,233,235,236,238,239,241,242,244,245,247,248,250,251,253,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*6.*/0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,32,33,33,34,34,35,35,36,36,37,37,38,38,39,39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,47,47,48,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,64,64,67,70,73,76,79,82,85,88,91,94,97,100,103,106,109,112,115,118,121,124,127,130,133,136,139,142,145,148,151,154,157,161,164,167,170,173,176,179,182,185,188,191,194,197,200,203,206,209,212,215,218,221,224,227,230,233,236,239,242,245,248,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*7.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95,97,99,101,103,105,107,109,111,113,115,117,119,121,123,125,127,129,131,133,135,137,139,141,143,145,147,149,151,153,155,157,159,161,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,203,205,207,209,211,213,215,217,219,221,223,225,227,229,231,233,235,237,239,241,243,245,247,249,251,253,255,
/*8.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,63,67,71,75,79,83,87,91,95,99,103,107,111,115,119,123,127,131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,195,199,203,207,211,215,219,223,227,231,235,239,243,247,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,171,167,163,159,155,151,147,143,139,135,131,127
};

unsigned char Cg[]={
/*0.*/0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
/*1.*/0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,255,255,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,222,220,218,216,214,212,210,208,206,204,202,200,198,196,194,192,190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,
/*2.*/0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,255,255,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,222,220,218,216,214,212,210,208,206,204,202,200,198,196,194,192,190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,
/*3.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*4.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*5.*/0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,170,166,162,158,154,150,146,142,138,134,130,126,122,118,114,110,106,102,98,94,90,85,81,77,73,69,65,61,57,53,49,45,41,37,33,29,25,21,17,13,9,5,0,
/*6.*/0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,170,166,162,158,154,150,146,142,138,134,130,126,122,118,114,110,106,102,98,94,90,85,81,77,73,69,65,61,57,53,49,45,41,37,33,29,25,21,17,13,9,5,0,
/*7.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*8.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,63,67,71,75,79,83,87,91,95,99,103,107,111,115,119,123,127,131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,195,199,203,207,211,215,219,223,227,231,235,239,243,247,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,171,167,163,159,155,151,147,143,139,135,131,127,123,119,115,111,107,103,99,95,91,87,83,79,75,71,67,63,59,55,51,47,43,39,35,31,27,23,19,15,11,7,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

unsigned char Cb[]={
/*0.*/0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
/*1.*/255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
/*2.*/255,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,222,220,218,216,214,212,210,208,206,204,202,200,198,196,194,192,190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*3.*/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255,
/*4.*/0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255,255,250,246,242,238,234,230,226,222,218,214,210,206,202,198,194,190,186,182,178,174,170,165,161,157,153,149,145,141,137,133,129,125,121,117,113,109,105,101,97,93,89,85,80,76,72,68,64,60,56,52,48,44,40,36,32,28,24,20,16,12,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255,
/*5.*/255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,127,125,123,121,119,117,115,113,111,109,107,105,103,101,99,97,95,93,91,89,87,85,83,81,79,77,75,73,71,69,67,65,63,61,59,57,55,53,51,49,47,45,43,41,39,37,35,33,31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*6.*/255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,252,249,246,243,240,237,234,231,228,225,222,219,216,213,210,207,204,201,198,195,192,189,186,183,180,177,174,171,168,165,162,158,155,152,149,146,143,140,137,134,131,128,125,122,119,116,113,110,107,104,101,98,95,92,89,86,83,80,77,74,71,68,64,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*7.*/255,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,222,220,218,216,214,212,210,208,206,204,202,200,198,196,194,192,190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*8.*/131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,195,199,203,207,211,215,219,223,227,231,235,239,243,247,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,171,167,163,159,155,151,147,143,139,135,131,127,123,119,115,111,107,103,99,95,91,87,83,79,75,71,67,63,59,55,51,47,43,39,35,31,27,23,19,15,11,7,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void get_pal_color(int pal, ru8 v, ru8 *r, ru8 *g, ru8 *b)
{
    *r = Cr[pal*256+v];
    *g = Cg[pal*256+v];
    *b = Cb[pal*256+v];
}

////////////////////////////////////////////////////////////////////////////////
/// test module functions
////////////////////////////////////////////////////////////////////////////////

int rtk_test_default(CParamArray *pa)
{
    printf("default test routine\n");
    return 0;
}

void rtk_print_basic_help(int argc, char *argv[],
                          RTK_TestFunctionArray fa[],
                          CParamArray &pa)
{
    int     i, j, k;
    int     tab = 30;

    fmt::print("\n");
    fmt::print("-------------------------- ");
    fmt::print_colored(fmt::GREEN, "basic usage");
    fmt::print(" --------------------------\n");

    fmt::print_colored(fmt::BLUE, "    -f"); fmt::print("              config file\n");
    fmt::print_colored(fmt::BLUE, "    -h"); fmt::print("              print usage help\n");
    fmt::print_colored(fmt::BLUE, "    -dbg_level"); fmt::print("      [0/1/2/3/4/5] debug level\n");
    fmt::print("                        1 - Error\n");
    fmt::print("                        2 - Warning\n");
    fmt::print("                        3 - Info\n");
    fmt::print("                        4 - Trace\n");
    fmt::print("                        5 - Normal\n");
    fmt::print_colored(fmt::BLUE, "    -act"); fmt::print("            [s] test module name\n");

    fmt::print("-----------------------------------------------------------------\n");
    fmt::print("\n");

    fmt::print("---------------------------- ");
    fmt::print_colored(fmt::GREEN, "modules");
    fmt::print(" ----------------------------\n");

    i=0;
    while( fa[i].f != NULL ) {
        fmt::print_colored(fmt::BLUE, "    {}", fa[i].name);

        j = strlen(fa[i].name);
        for(k=j; k<tab; k++) fmt::print(" ");

        fmt::print(" : {}\n", fa[i].desc);

        i++;
    }

    fmt::print("-----------------------------------------------------------------\n");
    fmt::print("\n");
}


int rtk_test_main(int argc, char *argv[],
                  RTK_TestFunctionArray fa[], CParamArray &pa)
{
    string          act;
    int             dbg_level = 4;
    int             i, j=0;
    rtk::ru64       t1=0, t2=0;
    int             ret=-1;
    string          fname;
    StringArray     sa1, sa2;

    // set signal handler
    dbg_stacktrace_setup();

    // parse input arguments
    if( argc <= 1 ) {
        ret = 0;
        goto RTK_TEST_MAIN_PRINT;
    }

    for(i=0; i<argc; i++) {
        // get config file
        if( strcmp(argv[i], "-f") == 0 ) {
            fname = argv[++i];
        }
        // print usage
        else if( strcmp(argv[i], "-h") == 0 ) {
            ret = 0;
            goto RTK_TEST_MAIN_PRINT;
        }
        // debug level
        else if( strcmp(argv[i], "-dbg_level") == 0 ) {
            dbg_level = atoi(argv[++i]);
            dbg_set_level(dbg_level);
        }
    }

    // load config file
    if( fname.length() > 0 )
        pa.load(fname + ".ini");

    // parse input argument again
    pa.set_args(argc, argv);

    // save input arguments to file
    sa1 = path_split(argv[0]);
    sa2 = path_splitext(sa1[1]);
    save_arguments(argc, argv, sa2[0]);

    // print all settings
    pa.print();

    // test actions
    act = "test_default";
    pa.s("act", act);

    // call test function
    i = 0; j = 0;
    while( fa[i].f != NULL ) {
        if( strcmp(act.c_str(), "test_default") == 0 ) {
            rtk_test_default(&pa);
            break;
        } else if( strcmp(act.c_str(), fa[i].name) == 0 ) {

            // run routine
            t1 = tm_get_us();
            ret = fa[i].f(&pa);
            t2 = tm_get_us();
            j = 1;

            break;
        }

        i++;
    }

    if( j == 0 ) {
        dbg_pe("Input arguments error!");
        goto RTK_TEST_MAIN_PRINT;
    }

    // print running time
    printf("\n---------------------------------------------------------\n");
    printf("run time = %g sec (%g min)\n",
                1.0*(t2-t1)/1000000.0,         /* sec */
                1.0*(t2-t1)/60000000.0);       /* min */
    printf("---------------------------------------------------------\n");

    goto RTK_TEST_MAIN_RET;

RTK_TEST_MAIN_PRINT:
    // print basic arguments
    rtk_print_basic_help(argc, argv, fa, pa);

    // print user provided help
    i=0;
    while( fa[i].f != NULL ) {
        if( strcmp(fa[i].name, "print_help") == 0 ) {
            ret = fa[i].f(&pa);
            break;
        }
        i++;
    }

RTK_TEST_MAIN_RET:
    return ret;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void rtk_print_basic_help_ex(int argc, char *argv[],
                          RTK_TestModuleArray ma[],
                          CParamArray &pa)
{
    int     i, j, k;
    int     tab = 30;

    fmt::print("\n");
    fmt::print("-------------------------- ");
    fmt::print_colored(fmt::GREEN, "basic usage");
    fmt::print(" --------------------------\n");

    fmt::print_colored(fmt::BLUE, "    -f"); fmt::print("              config file\n");
    fmt::print_colored(fmt::BLUE, "    -h"); fmt::print("              print usage help\n");
    fmt::print_colored(fmt::BLUE, "    -dbg_level"); fmt::print("      [0/1/2/3/4/5] debug level\n");
    fmt::print("                        1 - Error\n");
    fmt::print("                        2 - Warning\n");
    fmt::print("                        3 - Info\n");
    fmt::print("                        4 - Trace\n");
    fmt::print("                        5 - Normal\n");
    fmt::print_colored(fmt::BLUE, "    -act"); fmt::print("            [s] test module name\n");

    fmt::print("-----------------------------------------------------------------\n");
    fmt::print("\n");

    fmt::print("---------------------------- ");
    fmt::print_colored(fmt::GREEN, "modules");
    fmt::print(" ----------------------------\n");

    i=0;
    while( ma[i].m != NULL ) {
        fmt::print_colored(fmt::BLUE, "    {}", ma[i].name);

        j = strlen(ma[i].name);
        for(k=j; k<tab; k++) fmt::print(" ");

        fmt::print(" : {}\n", ma[i].desc);

        i++;
    }

    fmt::print("-----------------------------------------------------------------\n");
    fmt::print("\n");
}

int rtk_test_main(int argc, char *argv[], RTK_TestModuleArray ma[])
{
    CParamArray     *pa;
    int             isHelp = 0;
    string          act = "";
    int             dbg_level = 4;
    int             i, j=0;
    rtk::ru64       t1=0, t2=0;
    int             ret=-1;
    string          fname;
    StringArray     sa1, sa2;

    // set signal handler
    dbg_stacktrace_setup();

    // create a gobal ParameterArray obj
    pa = pa_create();

    // parse input arguments
    if( argc <= 1 ) {
        ret = 0;
        goto RTK_TEST_MAIN_PRINT;
    }

    for(i=0; i<argc; i++) {
        // get config file
        if( strcmp(argv[i], "-f") == 0 ) {
            if( i+1 < argc )
                fname = argv[++i];
            else
                goto RTK_TEST_MAIN_PRINT;
        }
        // print usage
        else if( strcmp(argv[i], "-h") == 0 ) {
            ret = 0;
            isHelp = 1;
        }
        // debug level
        else if( strcmp(argv[i], "-dbg_level") == 0 ) {
            if( i+1 < argc ) {
                dbg_level = atoi(argv[++i]);
                dbg_set_level(dbg_level);
            } else
                goto RTK_TEST_MAIN_PRINT;
        }
        // action
        else if( strcmp(argv[i], "-act") == 0 ) {
            if( i+1 < argc ) {
                act = argv[++i];
            } else
                goto RTK_TEST_MAIN_PRINT;
        }
    }

    if( isHelp ) {
        goto RTK_TEST_MAIN_PRINT;
    }


    // load config file
    if( fname.length() > 0 )
        pa->load(fname + ".ini");

    // set argc, argv to pa
    pa->set_i("argc", argc);
    pa->set_p("argv", argv);

    // parse input argument again
    pa->set_args(argc, argv);

    // save input arguments to file
    sa1 = path_split(argv[0]);
    sa2 = path_splitext(sa1[1]);
    save_arguments(argc, argv, sa2[0]);

    // print all settings
    pa->print();

    // call test function
    i = 0; j = 0;
    while( ma[i].m != NULL ) {
        if( strcmp(act.c_str(), ma[i].name) == 0 ) {

            // run routine
            t1 = tm_get_us();
            ret = ma[i].m->run(pa);
            t2 = tm_get_us();

            j = 1;

            break;
        }

        i++;
    }

    if( j == 0 ) {
        dbg_pe("Input arguments error!");
        goto RTK_TEST_MAIN_PRINT;
    }

    // print running time
    fmt::print("\n---------------------------------------------------------\n");
    fmt::print_colored(fmt::BLUE, "run time");
    fmt::print(" = {} sec ({} min)\n",
                1.0*(t2-t1)/1000000.0,         /* sec */
                1.0*(t2-t1)/60000000.0);       /* min */
    fmt::print("---------------------------------------------------------\n");

    goto RTK_TEST_MAIN_RET;

RTK_TEST_MAIN_PRINT:
    // print basic arguments
    rtk_print_basic_help_ex(argc, argv, ma, *pa);

    if( act.size() == 0 ) return ret;

    // call module help function
    i = 0;
    while( ma[i].m != NULL ) {
        if( strcmp(act.c_str(), ma[i].name) == 0 ) {

            fmt::print("\n");
            fmt::print("=======================================\n");
            fmt::print(">> Module: ");
            fmt::print_colored(fmt::BLUE, "{}\n", ma[i].name);
            fmt::print("=======================================\n");

            // run user provided help function
            ma[i].m->help(pa);

            fmt::print("\n");

            break;
        }

        i++;
    }

RTK_TEST_MAIN_RET:
    return ret;
}


} // end of namespace rtk
