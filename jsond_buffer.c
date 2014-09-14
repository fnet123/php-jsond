/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Jakub Zelenka <bukka@php.net>                                |
  +----------------------------------------------------------------------+
*/

#include "php_jsond_buffer.h"
#include "php_jsond.h"

void php_json_buffer_init(php_json_buffer *buf) /* {{{ */
{
	buf->dbuf = NULL;
	buf->left = PHP_JSON_BUFFER_STATIC_SIZE;
	buf->ptr = &buf->sbuf[0];
	buf->dsize = 0;
}
/* }}} */

void php_json_buffer_flush(php_json_buffer *buf, size_t pre_alloc_size) /* {{{ */
{
	size_t static_size = PHP_JSON_BUFFER_STATIC_SIZE - buf->left;
	size_t size = static_size + pre_alloc_size + 1;
	if (buf->dbuf) {
		buf->dbuf = erealloc(buf->dbuf, size);
	} else {
		buf->dbuf = emalloc(size);
	}
	memcpy(buf->dbuf + buf->dsize, &buf->sbuf[0], static_size);
	buf->dsize += static_size;
	buf->ptr = &buf->sbuf[0];
	buf->left = PHP_JSON_BUFFER_STATIC_SIZE;
}
/* }}} */

void php_json_buffer_append_char(php_json_buffer *buf, char c) /* {{{ */
{
	if (--buf->left == 0) {
		php_json_buffer_flush(buf, PHP_JSON_BUFFER_EXTRA_ALLOC_SIZE);
	}
	*buf->ptr = c;
	buf->ptr++;
}
/* }}} */

void php_json_buffer_append_stringl(php_json_buffer *buf, const char *str, size_t len) /* {{{ */
{
	if (len > PHP_JSON_BUFFER_STATIC_SIZE || buf->left - (int) len <= 0) {
		php_json_buffer_flush(buf, len + PHP_JSON_BUFFER_EXTRA_ALLOC_SIZE);
		memcpy(buf->dbuf + buf->dsize, str, len);
	} else {
		memcpy(buf->ptr, str, len);
		buf->ptr += len;
	}
}
/* }}} */

void php_json_buffer_append_long(php_json_buffer *buf, long l) /* {{{ */
{
	char str[PHP_JSON_INT_MAX_LENGTH + 1];
	char *p = &str[PHP_JSON_INT_MAX_LENGTH];
	size_t len = 0;
	zend_bool negative;

	if (l < 0) {
		negative = 1;
		l = -l;
	} else {
		negative = 0;
	}

	do {
		*(p--) = (char) (l % 10) + '0';
		l /= 10;
		len++;
	} while (l > 0);

	if (negative) {
		*p = '-';
		len++;
	} else {
		p++;
	}

	php_json_buffer_append_stringl(buf, p, len);
}
/* }}} */