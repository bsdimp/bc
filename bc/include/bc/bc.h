#ifndef BC_H
#define BC_H

#include <stdint.h>

typedef void (*BcFreeFunc)(void*);
typedef int (*BcCmpFunc)(void*, void*);

#define BC_BASE_MAX_DEF (99)
#define BC_DIM_MAX_DEF (2048)
#define BC_SCALE_MAX_DEF (99)
#define BC_STRING_MAX_DEF (1024)

typedef enum BcStatus {

	BC_STATUS_SUCCESS,

	BC_STATUS_INVALID_OPTION,
	BC_STATUS_MALLOC_FAIL,
	BC_STATUS_INVALID_PARAM,

	BC_STATUS_NO_LIMIT,
	BC_STATUS_INVALID_LIMIT,

	BC_STATUS_VM_FILE_ERR,
	BC_STATUS_VM_FILE_READ_ERR,
	BC_STATUS_VM_DIVIDE_BY_ZERO,
	BC_STATUS_VM_NEG_SQRT,
	BC_STATUS_VM_MISMATCHED_PARAMS,
	BC_STATUS_VM_UNDEFINED_FUNC,
	BC_STATUS_VM_FILE_NOT_EXECUTABLE,
	BC_STATUS_VM_SIGACTION_FAIL,

	BC_STATUS_LEX_INVALID_TOKEN,
	BC_STATUS_LEX_NO_STRING_END,
	BC_STATUS_LEX_NO_COMMENT_END,
	BC_STATUS_LEX_EOF,

	BC_STATUS_PARSE_INVALID_TOKEN,
	BC_STATUS_PARSE_INVALID_EXPR,
	BC_STATUS_PARSE_INVALID_PRINT,
	BC_STATUS_PARSE_INVALID_FUNC,
	BC_STATUS_PARSE_NO_AUTO,
	BC_STATUS_PARSE_QUIT,
	BC_STATUS_PARSE_EOF,
	BC_STATUS_PARSE_BUG,

} BcStatus;

BcStatus bc_main(int argc, char* argv[]);

BcStatus bc_limits();

void bc_error(BcStatus status);
void bc_error_file(BcStatus status, const char* file, uint32_t line);

extern int bc_interactive;
extern int bc_standard;
extern int bc_warn;

extern int bc_had_sigint;

#endif // BC_H
