/*
 * *****************************************************************************
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018-2020 Gavin D. Howard and contributors.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
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
 * *****************************************************************************
 *
 * All bc status codes.
 *
 */

#ifndef BC_STATUS_H
#define BC_STATUS_H

#include <stdint.h>

#ifndef BC_ENABLED
#define BC_ENABLED (1)
#endif // BC_ENABLED

#ifndef DC_ENABLED
#define DC_ENABLED (1)
#endif // DC_ENABLED

typedef enum BcStatus {

	BC_STATUS_SUCCESS = 0,
	BC_STATUS_ERROR_MATH,
	BC_STATUS_ERROR_PARSE,
	BC_STATUS_ERROR_EXEC,
	BC_STATUS_ERROR_FATAL,
	BC_STATUS_EOF,
	BC_STATUS_QUIT,

} BcStatus;

typedef enum BcError {

	BC_ERROR_MATH_NEGATIVE,
	BC_ERROR_MATH_NON_INTEGER,
	BC_ERROR_MATH_OVERFLOW,
	BC_ERROR_MATH_DIVIDE_BY_ZERO,

	BC_ERROR_FATAL_ALLOC_ERR,
	BC_ERROR_FATAL_IO_ERR,
	BC_ERROR_FATAL_FILE_ERR,
	BC_ERROR_FATAL_BIN_FILE,
	BC_ERROR_FATAL_PATH_DIR,
	BC_ERROR_FATAL_OPTION,
	BC_ERROR_FATAL_OPTION_NO_ARG,
	BC_ERROR_FATAL_OPTION_ARG,

	BC_ERROR_EXEC_IBASE,
	BC_ERROR_EXEC_OBASE,
	BC_ERROR_EXEC_SCALE,
	BC_ERROR_EXEC_READ_EXPR,
	BC_ERROR_EXEC_REC_READ,
	BC_ERROR_EXEC_TYPE,

	BC_ERROR_EXEC_STACK,

	BC_ERROR_EXEC_PARAMS,
	BC_ERROR_EXEC_UNDEF_FUNC,
	BC_ERROR_EXEC_VOID_VAL,

	BC_ERROR_PARSE_EOF,
	BC_ERROR_PARSE_CHAR,
	BC_ERROR_PARSE_STRING,
	BC_ERROR_PARSE_COMMENT,
	BC_ERROR_PARSE_TOKEN,
#if BC_ENABLED
	BC_ERROR_PARSE_EXPR,
	BC_ERROR_PARSE_EMPTY_EXPR,
	BC_ERROR_PARSE_PRINT,
	BC_ERROR_PARSE_FUNC,
	BC_ERROR_PARSE_ASSIGN,
	BC_ERROR_PARSE_NO_AUTO,
	BC_ERROR_PARSE_DUP_LOCAL,
	BC_ERROR_PARSE_BLOCK,
	BC_ERROR_PARSE_RET_VOID,
	BC_ERROR_PARSE_REF_VAR,

	BC_ERROR_POSIX_NAME_LEN,
	BC_ERROR_POSIX_COMMENT,
	BC_ERROR_POSIX_KW,
	BC_ERROR_POSIX_DOT,
	BC_ERROR_POSIX_RET,
	BC_ERROR_POSIX_BOOL,
	BC_ERROR_POSIX_REL_POS,
	BC_ERROR_POSIX_MULTIREL,
	BC_ERROR_POSIX_FOR,
	BC_ERROR_POSIX_EXP_NUM,
	BC_ERROR_POSIX_REF,
	BC_ERROR_POSIX_VOID,
	BC_ERROR_POSIX_BRACE,
#endif // BC_ENABLED

	BC_ERROR_NELEMS,

#if BC_ENABLED
	BC_ERROR_POSIX_START = BC_ERROR_POSIX_NAME_LEN,
	BC_ERROR_POSIX_END = BC_ERROR_POSIX_BRACE,
#endif // BC_ENABLED

} BcError;

#define BC_ERR_IDX_MATH (0)
#define BC_ERR_IDX_PARSE (1)
#define BC_ERR_IDX_EXEC (2)
#define BC_ERR_IDX_FATAL (3)
#define BC_ERR_IDX_NELEMS (4)

#if BC_ENABLED
#define BC_ERR_IDX_WARN (BC_ERR_IDX_NELEMS)
#endif // BC_ENABLED

#define BC_UNUSED(e) ((void) (e))

#ifndef BC_LIKELY
#define BC_LIKELY(e) (e)
#endif // BC_LIKELY

#ifndef BC_UNLIKELY
#define BC_UNLIKELY(e) (e)
#endif // BC_UNLIKELY

#define BC_ERR(e) BC_UNLIKELY(e)
#define BC_NO_ERR(s) BC_LIKELY(s)

#ifndef BC_DEBUG_CODE
#define BC_DEBUG_CODE (0)
#endif // BC_DEBUG_CODE

#if __STDC_VERSION__ >= 201100L
#include <stdnoreturn.h>
#define BC_NORETURN _Noreturn
#else // __STDC_VERSION__
#define BC_NORETURN
#define BC_MUST_RETURN
#endif // __STDC_VERSION__

#ifdef __clang__
#define BC_FALLTHROUGH __attribute__((fallthrough));
#else // __clang__
#define BC_FALLTHROUGH
#endif // __clang__

// Workarounds for AIX's POSIX incompatibility.
#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif // SIZE_MAX
#ifndef UINTMAX_C
#define UINTMAX_C __UINTMAX_C
#endif // UINTMAX_C
#ifndef UINT32_C
#define UINT32_C __UINT32_C
#endif // UINT32_C
#ifndef UINT_FAST32_MAX
#define UINT_FAST32_MAX __UINT_FAST32_MAX__
#endif // UINT_FAST32_MAX
#ifndef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif // UINT16_MAX
#ifndef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX __SIG_ATOMIC_MAX__
#endif // SIG_ATOMIC_MAX

#endif // BC_STATUS_H
