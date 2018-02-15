/*
 * Copyright 2018 Contributors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 * A special license exemption is granted to the Toybox project to use this
 * source under the following BSD 0-Clause License:
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *******************************************************************************
 *
 * Code for the number type.
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <bc.h>
#include <num.h>

static BcStatus bc_num_unary(BcNum* a, BcNum* b, size_t scale,
                             BcUnaryFunc op, size_t req);

static BcStatus bc_num_binary(BcNum* a, BcNum* b, BcNum* c, size_t scale,
                              BcBinaryFunc op, size_t req);

static BcStatus bc_num_alg_a(BcNum* a, BcNum* b, BcNum* c, size_t scale);
static BcStatus bc_num_alg_s(BcNum* a, BcNum* b, BcNum* c, size_t scale);
static BcStatus bc_num_alg_m(BcNum* a, BcNum* b, BcNum* c, size_t scale);
static BcStatus bc_num_alg_d(BcNum* a, BcNum* b, BcNum* c, size_t scale);
static BcStatus bc_num_alg_mod(BcNum* a, BcNum* b, BcNum* c, size_t scale);
static BcStatus bc_num_alg_rem(BcNum* a, BcNum* b, BcNum* c);
static BcStatus bc_num_alg_p(BcNum* a, BcNum* b, BcNum* c, size_t scale);

static BcStatus bc_num_sqrt_newton(BcNum* a, BcNum* b, size_t scale);

static bool bc_num_strValid(const char* val, size_t base);

static BcStatus bc_num_parseDecimal(BcNum *n, const char* val, size_t scale);
static BcStatus bc_num_parseLowBase(BcNum* n, const char* val,
                                     size_t base, size_t scale);
static BcStatus bc_num_parseHighBase(BcNum* n, const char* val,
                                      size_t base, size_t scale);

static BcStatus bc_num_printDecimal(BcNum* n, FILE* f);
static BcStatus bc_num_printLowBase(BcNum* n, size_t base, FILE* f);
static BcStatus bc_num_printHighBase(BcNum* n, size_t base, FILE* f);
static BcStatus bc_num_printHighestBase(BcNum* n, size_t base, FILE* f);

static BcStatus bc_num_removeLeadingZeros(BcNum* n);

BcStatus bc_num_construct(BcNum* n, size_t request) {

  if (!n || !request) return BC_STATUS_INVALID_PARAM;

  memset(n, 0, sizeof(BcNum));

  n->num = malloc(request);

  if (!n->num) {
    return BC_STATUS_MALLOC_FAIL;
  }

  n->unused = request;

  return BC_STATUS_SUCCESS;
}

BcStatus bc_num_expand(BcNum* n, size_t request) {

  if (!n || !request) return BC_STATUS_INVALID_PARAM;

  if (request > n->len + n->unused) {

    size_t extra;
    char* temp;

    extra = request - (n->len + n->unused);

    temp = realloc(n->num, request);

    if (!temp) {
      return BC_STATUS_MALLOC_FAIL;
    }

    n->num = temp;

    n->unused += extra;
  }

  return BC_STATUS_SUCCESS;
}

void bc_num_destruct(BcNum* n) {

  if (!n) return;

  if (n->num) free(n->num);

  memset(n, 0, sizeof(BcNum));
}

BcStatus bc_num_copy(BcNum* d, BcNum* s) {

  BcStatus status;

  if (!d || !s || d == s) return BC_STATUS_INVALID_PARAM;

  status = bc_num_expand(d, s->len + s->unused);

  if (status) return status;

  d->len = s->len;
  d->neg = s->neg;
  d->radix = s->radix;

  memcpy(d->num, s->num, sizeof(char) * d->len);

  return BC_STATUS_SUCCESS;
}

BcStatus bc_num_parse(BcNum* n, const char* val,
                      size_t base, size_t scale)
{
  BcStatus status;

  if (!n || !val) return BC_STATUS_INVALID_PARAM;

  if (base < BC_NUM_MIN_BASE || base > BC_NUM_MAX_INPUT_BASE)
    return BC_STATUS_EXEC_INVALID_IBASE;

  if (!bc_num_strValid(val, base)) return BC_STATUS_MATH_INVALID_STRING;

  if (base == 10) {
    status = bc_num_parseDecimal(n, val, scale);
  }
  else if (base < 10) {
    status = bc_num_parseLowBase(n, val, base, scale);
  }
  else {
    status = bc_num_parseHighBase(n, val, base, scale);
  }

  return status;
}

BcStatus bc_num_print(BcNum* n, size_t base) {
  return bc_num_fprint(n, base, stdout);
}

BcStatus bc_num_fprint(BcNum* n, size_t base, FILE* f) {

  BcStatus status;

  if (!n || !f) return BC_STATUS_INVALID_PARAM;

  if (base < BC_NUM_MIN_BASE || base > BC_NUM_MAX_OUTPUT_BASE)
    return BC_STATUS_EXEC_INVALID_OBASE;

  if (base == 10) {
    status = bc_num_printDecimal(n, f);
  }
  else if (base < 10) {
    status = bc_num_printLowBase(n, base, f);
  }
  else if (base <= 16) {
    status = bc_num_printHighBase(n, base, f);
  }
  else {
    status = bc_num_printHighestBase(n, base, f);
  }

  return status;
}

BcStatus bc_num_long(BcNum* n, long* result) {

  size_t i;
  unsigned long temp;
  unsigned long prev;

  if (!n || !result) return BC_STATUS_INVALID_PARAM;

  if (n->radix != n->len) return BC_STATUS_MATH_NON_INTEGER;

  temp = 0;

  for (i = 0; i < n->len; ++i) {

    prev = temp;

    temp *= 10;
    temp += n->num[i];

    if (temp < prev) return BC_STATUS_MATH_OVERFLOW;
  }

  *result = temp;

  return BC_STATUS_SUCCESS;
}

BcStatus bc_num_ulong(BcNum* n, unsigned long* result) {

  size_t i;
  unsigned long prev;

  if (!n || !result) return BC_STATUS_INVALID_PARAM;

  if (n->radix != n->len) return BC_STATUS_MATH_NON_INTEGER;

  if (n->neg) return BC_STATUS_MATH_NEGATIVE;

  *result = 0;

  for (i = 0; i < n->len; ++i) {

    prev = *result;

    *result *= 10;
    *result += n->num[i];

    if (*result < prev) return BC_STATUS_MATH_OVERFLOW;
  }

  return BC_STATUS_SUCCESS;
}

BcStatus bc_num_add(BcNum* a, BcNum* b, BcNum* result, size_t scale) {
  return bc_num_binary(a, b, result, scale, bc_num_alg_a, a->len + b->len + 1);
}

BcStatus bc_num_sub(BcNum* a, BcNum* b, BcNum* result, size_t scale) {
  return bc_num_binary(a, a, result, scale, bc_num_alg_s, a->len + b->len + 1);
}

BcStatus bc_num_mul(BcNum* a, BcNum* b, BcNum* result, size_t scale) {
  return bc_num_binary(a, b, result, scale, bc_num_alg_m, a->len + b->len);
}

BcStatus bc_num_div(BcNum* a, BcNum* b, BcNum* result, size_t scale) {
  return bc_num_binary(a, b, result, scale, bc_num_alg_d, a->len + b->len);
}

BcStatus bc_num_mod(BcNum* a, BcNum* b, BcNum* result, size_t scale) {
  return bc_num_binary(a, b, result, scale, bc_num_alg_mod, a->len + b->len);
}

BcStatus bc_num_pow(BcNum* a, BcNum* b, BcNum* result, size_t scale) {
  return bc_num_binary(a, b, result, scale, bc_num_alg_p, a->len * b->len);
}

BcStatus bc_num_sqrt(BcNum* a, BcNum* result, size_t scale) {
  return bc_num_unary(a, result, scale, bc_num_sqrt_newton,
                      a->radix + (a->len - a->radix) * 2);
}

bool bc_num_isInteger(BcNum* num) {

  BcNum* n;

  n = (BcNum*) num;

  if (!n) return false;

  return n->radix == n->len;
}

int bc_num_compare(BcNum* a, BcNum* b) {

  BcNum* a2;
  BcNum* b2;
  size_t i;
  size_t max;
  size_t min;
  char* max_num;
  char* min_num;
  bool a_max;

  a2 = (BcNum*) a;
  b2 = (BcNum*) b;

  if (!a2) {

    if (b2== NULL) {
      return 0;
    }
    else {
      return b2->neg ? 1 : -1;
    }
  }
  else if (!b2) {
    return a2->neg ? -1 : 1;
  }

  if (a2->radix > b2->radix) {
    return 1;
  }
  else if (b2->radix > a2->radix) {
    return -1;
  }

  for (i = 0; i < a2->radix; ++i) {

    char c;

    c = a2->num[i] - b2->num[i];

    if (c) return c;
  }

  a_max = a2->len > b2->len;

  if (a_max) {

    max = a2->len - a2->radix;
    min = b2->len - b2->radix;

    max_num = a2->num + a2->radix;
    min_num = b2->num + b2->radix;
  }
  else {

    max = b2->len - b2->radix;
    min = a2->len - a2->radix;

    max_num = b2->num + b2->radix;
    min_num = a2->num + a2->radix;
  }

  for (i = 0; i < min; ++i) {

    char c;

    c = max_num[i] - min_num[i];

    if (c) return a_max ? c : -c;
  }

  for (; i < max; ++i) {
    if (max_num[i]) return a_max ? 1 : -1;
  }

  return 0;
}

static BcStatus bc_num_unary(BcNum* a, BcNum* b, size_t scale,
                               BcUnaryFunc op, size_t req)
{
  BcStatus status;
  BcNum a2;
  BcNum* ptr_a;

  if (!a || !b || !op) return BC_STATUS_INVALID_PARAM;

  if (b == a) {

    memcpy(&a2, b, sizeof(BcNum));
    ptr_a = &a2;

    status = bc_num_construct(b, req);
  }
  else {
    ptr_a = a;
    status = bc_num_expand(b, req);
  }

  if (status) return status;

  status = op(ptr_a, b, scale);

  if (b == a) {
    bc_num_destruct(&a2);
  }

  return status;
}

static BcStatus bc_num_binary(BcNum* a, BcNum* b, BcNum* c,
                               size_t scale, BcBinaryFunc op, size_t req)
{
  BcStatus status;
  BcNum a2;
  BcNum b2;
  BcNum* ptr_a;
  BcNum* ptr_b;
  bool init;

  if (!a || !b || !c || !op) return BC_STATUS_INVALID_PARAM;

  init = false;

  if (c == a) {
    memcpy(&a2, c, sizeof(BcNum));
    ptr_a = &a2;
    init = true;
  }
  else {
    ptr_a = a;
  }

  if (c == b) {

    if (c == a) {
      ptr_b = ptr_a;
    }
    else {
      memcpy(&b2, c, sizeof(BcNum));
      ptr_b = &b2;
      init = true;
    }
  }
  else {
    ptr_b = b;
  }

  if (init) {
    status = bc_num_construct(c, req);
  }
  else {
    status = bc_num_expand(c, req);
  }

  if (status) return status;

  status = op(ptr_a, ptr_b, c, scale);

  if (c == a) {
    bc_num_destruct(&a2);
  }
  else if (c == b) {
    bc_num_destruct(&b2);
  }

  return status;
}

static BcStatus bc_num_alg_a(BcNum* a, BcNum* b, BcNum* c, size_t scale) {

  char* ptr;
  char* ptr_a;
  char* ptr_b;
  char* ptr_c;
  size_t scale_a;
  size_t scale_b;
  size_t i;
  size_t min;
  char carry;

  memset(c->num, 0, (c->len + c->unused) * sizeof(char));

  c->radix = BC_MAX(a->radix, b->radix) + 1;

  scale_a = BC_NUM_SCALE(a);
  scale_b = BC_NUM_SCALE(b);

  scale = BC_MAX(scale_a, scale_b);

  min = BC_MIN(scale_a, scale_b);

  c->unused -= c->len - (c->radix + scale);
  c->len = c->radix + scale;

  ptr_a = a->num + a->radix;
  ptr_b = b->num + b->radix;
  ptr_c = c->num + c->radix;

  ptr = scale_a > scale_b ? ptr_a : ptr_b;

  i = scale - 1;

  while (i >= min) {
    ptr_c[i] = ptr[i];
    --i;
  }

  carry = 0;

  for (; i < scale; --i) {

    ptr_c[i] = ptr_a[i] + ptr_b[i] + carry;

    carry = 0;

    while (ptr_c[i] >= 10) {
      carry += 1;
      ptr_c[i] -= 10;
    }
  }

  if (a->radix == c->radix - 1) {

    min = b->radix;
    scale = a->radix - min;
    i = min - 1;

    ptr_a = a->num + scale;
    ptr_b = b->num;
    ptr_c = c->num + (c->radix - min);

    ptr = a->num;
  }
  else {

    min = a->radix;
    scale = b->radix - min;
    i = min - 1;

    ptr_a = a->num;
    ptr_b = b->num + scale;
    ptr_c = c->num + (c->radix - min);

    ptr = b->num;
  }

  for (; i < min; --i) {

    ptr_c[i] = ptr_a[i] + ptr_b[i] + carry;

    carry = 0;

    while (ptr_c[i] >= 10) {
      carry += 1;
      ptr_c[i] -= 10;
    }
  }

  --ptr_c;

  *ptr_c = carry;

  i = scale - 1;

  while (i < scale) {
    *ptr_c = ptr[i];
    --ptr_c;
    --i;
  }

  return bc_num_removeLeadingZeros(c);
}

static BcStatus bc_num_alg_s(BcNum* a, BcNum* b, BcNum* c, size_t scale) {

  scale = BC_MAX(BC_NUM_SCALE(a), BC_NUM_SCALE(b));

}

static BcStatus bc_num_alg_m(BcNum* a, BcNum* b, BcNum* c, size_t scale) {

  size_t scale_a;
  size_t scale_b;

  scale_a = BC_NUM_SCALE(a);
  scale_b = BC_NUM_SCALE(b);

  scale = BC_MAX(scale, scale_a);
  scale = BC_MAX(scale, scale_b);
  scale = BC_MIN(scale_a + scale_b, scale);

}

static BcStatus bc_num_alg_d(BcNum* a, BcNum* b, BcNum* c, size_t scale) {

}

static BcStatus bc_num_alg_mod(BcNum* a, BcNum* b, BcNum* c, size_t scale) {

  if (scale == 0) {
    return bc_num_alg_rem(a, b, c);
  }

  // TODO: Compute a / b.

  scale = BC_MAX(scale + b->len - b->radix, a->len - a->radix);

  // TODO: Compute a - (a / b) * b.

  return BC_STATUS_SUCCESS;
}

static BcStatus bc_num_alg_rem(BcNum* a, BcNum* b, BcNum* c) {

}

static BcStatus bc_num_alg_p(BcNum* a, BcNum* b, BcNum* c, size_t scale) {

}

static BcStatus bc_num_sqrt_newton(BcNum* a, BcNum* b, size_t scale) {

}

static bool bc_num_strValid(const char* val, size_t base) {

  size_t len;
  size_t i;
  char c;
  char b;
  bool radix;

  radix = false;

  len = strlen(val);

  if (!len) return true;

  c = val[0];

  i = c == '-' || c == '+' ? 1 : 0;

  if (base <= 10) {

    b = base + '0';

    for (; i < len; ++i) {

      c = val[i];

      if (c == '.') {

        if (radix) return false;

        radix = true;

        continue;
      }

      if (c < '0' || c >= b) return false;
    }
  }
  else {

    b = base - 9 + 'A';

    for (; i < len; ++i) {

      c = val[i];

      if (c == '.') {

        if (radix) return false;

        radix = true;

        continue;
      }

      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= b))) return false;
    }
  }

  return true;
}

static BcStatus bc_num_parseDecimal(BcNum* n, const char* val, size_t scale) {

  BcStatus status;
  size_t len;
  char c;
  size_t i;
  const char* ptr;
  size_t radix;
  size_t inv_pow;
  char* num;

  len = strlen(val);

  n->len = 0;
  n->neg = false;
  n->radix = 0;

  if (len) {

    status = bc_num_expand(n, len);

    if (status) return status;
  }
  else {

    n->num = malloc(BC_NUM_DEF_SIZE);

    if (!n->num) return BC_STATUS_MALLOC_FAIL;

    memset(n->num, 0, sizeof(char) * BC_NUM_DEF_SIZE);

    return BC_STATUS_SUCCESS;
  }

  ptr = strchr(val, '.');

  if (ptr) radix = ptr - val;
  else radix = len;

  c = val[0];

  n->neg = c == '-';

  i = c == '-' || c == '+' ? 1 : 0;

  while (val[i] == '0') ++i;

  ptr = val + i;
  radix -= i;
  len -= i;

  for (i = 0; i < radix; ++i) {
    n->num[i] = BC_NUM_FROM_CHAR(ptr[i]);
    n->len += 1;
    n->unused -= 1;
  }

  ptr += radix + 1;

  n->radix = radix;

  if (i >= len) return BC_STATUS_SUCCESS;

  inv_pow = 0;

  while (inv_pow < scale && ptr[inv_pow] == '0') ++inv_pow;

  if (inv_pow >= scale || ptr[inv_pow] == '\0') {

    if (!n->len) n->neg = false;

    n->radix = n->len;

    return BC_STATUS_SUCCESS;
  }

  num = n->num + n->len;

  for (i = 0; i < inv_pow; ++i) {
    num[i] = 0;
    ++n->len;
    --n->unused;
  }

  c = ptr[i];

  while (i < scale && c != '\0') {

    num[i] = BC_NUM_FROM_CHAR(c);

    ++n->len;
    --n->unused;

    ++i;
    c = ptr[i];
  }

  return BC_STATUS_SUCCESS;
}

static BcStatus bc_num_parseLowBase(BcNum* n, const char* val,
                                     size_t base, size_t scale)
{
  const char* ptr;
  char* nptr;
  size_t len;
  size_t radix;
  size_t digits;
  char c;
  char carry;
  size_t pow;

  len = strlen(val);

  ptr = strchr(val, '.');

  if (ptr) {
    --ptr;
  }
  else {
    ptr = val + len - 1;
  }

  radix = ptr - val;

  nptr = n->num + radix;
  ++radix;

  c = 0;
  carry = 0;
  digits = 0;
  pow = 1;

  while (ptr >= val) {

    c += BC_NUM_FROM_CHAR(*ptr) * pow;
    --ptr;
    pow *= base;

    if (pow > 10) {

      --nptr;

      while (c >= 10) {
        *nptr += 1;
        c -= 10;
      }

      *(nptr + 1) += c;
      ++digits;

      carry = 1;
      c = 0;
      pow = 1;
    }
  }

  if (c) {
    *nptr += c;
    --nptr;
    ++digits;
  }

  if (nptr >= n->num) {
    memmove(n->num, nptr + 1, digits * sizeof(char));
  }

  // TODO: After radix.

  return BC_STATUS_SUCCESS;
}
static BcStatus bc_num_parseHighBase(BcNum* n, const char* val,
                                      size_t base, size_t scale)
{

}

static BcStatus bc_num_printDecimal(BcNum* n, FILE* f) {

  size_t i;

  if (n->len) {

    if (n->neg) fputc('-', f);

    for (i = 0; i < n->radix; ++i) fputc(BC_NUM_TO_CHAR(n->num[i]), f);

    if (i < n->len) {

      fputc('.', f);

      for (; i < n->len; ++i) fputc(BC_NUM_TO_CHAR(n->num[i]), f);
    }
  }
  else {
    fputc('0', f);
  }

  fputc('\n', f);

  return BC_STATUS_SUCCESS;
}

static BcStatus bc_num_printLowBase(BcNum* n, size_t base, FILE* f) {

  size_t size;
  char* buf;
  size_t i;

  size = BC_MAX(n->radix, n->len - n->radix) * ((10 * 2) / base);

  buf = malloc(size);

  if (!buf) return BC_STATUS_MALLOC_FAIL;

  i = 0;

  fputc('\n', f);

  free(buf);

  return BC_STATUS_SUCCESS;
}

static BcStatus bc_num_printHighBase(BcNum* n, size_t base, FILE* f) {

}

static BcStatus bc_num_printHighestBase(BcNum* n, size_t base, FILE* f) {

}

static BcStatus bc_num_removeLeadingZeros(BcNum* n) {

  size_t i;
  char* ptr;

  for (i = 0; n->num[i] == 0 && i < n->radix; ++i);

  if (i == n->radix) {

    n->unused += n->radix;
    n->len -= n->radix;
    n->radix = 0;

    return BC_STATUS_SUCCESS;
  }

  ptr = n->num + i;

  memmove(n->num, ptr, i * sizeof(char));

  return BC_STATUS_SUCCESS;
}
