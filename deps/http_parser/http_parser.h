/* Copyright 2009 Ryan Dahl <ry@tinyclouds.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE. 
 */
#ifndef http_parser_h
#define http_parser_h
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#  include <stddef.h>
#endif
#include <sys/types.h>

/* Compile with -DHTTP_PARSER_STRICT=0 to make less checks, but run
 * faster 
 */ 
#ifndef HTTP_PARSER_STRICT
# define HTTP_PARSER_STRICT 1
#else
# define HTTP_PARSER_STRICT 0
#endif

typedef struct http_parser http_parser;

/* Callbacks should return non-zero to indicate an error. The parse will
 * then halt execution.
 *
 * http_data_cb does not return data chunks. It will be call arbitrarally
 * many times for each string. E.G. you might get 10 callbacks for "on_path"
 * each providing just a few characters more data.
 */
typedef int (*http_data_cb) (http_parser*, const char *at, size_t length);
typedef int (*http_cb) (http_parser*);

/* Request Methods */
enum http_method
  { HTTP_DELETE    = 0x0002
  , HTTP_GET       = 0x0004
  , HTTP_HEAD      = 0x0008
  , HTTP_POST      = 0x0100
  , HTTP_PUT       = 0x0800
  };

struct http_parser {
  /** PRIVATE **/
  unsigned short state;
  unsigned short header_state;
  size_t header_index;

  char flags;

  ssize_t body_read;
  ssize_t content_length;

  const char *header_field_mark;
  size_t      header_field_size;
  const char *header_value_mark;
  size_t      header_value_size;
  const char *query_string_mark;
  size_t      query_string_size;
  const char *path_mark;
  size_t      path_size;
  const char *url_mark;
  size_t      url_size;
  const char *fragment_mark;
  size_t      fragment_size;

  /** READ-ONLY **/
  unsigned short status_code; /* responses only */
  enum http_method method;    /* requests only */
  unsigned short http_major;
  unsigned short http_minor;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */

  /* an ordered list of callbacks */

  http_cb      on_message_begin;

  /* requests only */
  http_data_cb on_path;
  http_data_cb on_query_string;
  http_data_cb on_url;
  http_data_cb on_fragment;

  http_data_cb on_header_field;
  http_data_cb on_header_value;
  http_cb      on_headers_complete;
  http_data_cb on_body;
  http_cb      on_message_complete;
};

void http_parser_init(http_parser *parser);
size_t http_parse_requests(http_parser *parser, const char *data, size_t len);
size_t http_parse_responses(http_parser *parser, const char *data, size_t len);
/* Call this in the on_headers_complete or on_message_complete callback to
 * determine if this will be the last message on the connection.  
 * If you are the server, respond with the "Connection: close" header
 * if you are the client, close the connection.
 */
int http_should_keep_alive(http_parser *parser);

#ifdef __cplusplus
}
#endif
#endif
