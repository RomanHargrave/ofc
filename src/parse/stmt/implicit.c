#include "../parse.h"



unsigned parse_stmt_implicit(
	const sparse_t* src, const char* ptr,
	parse_stmt_t* stmt)
{
	unsigned i = parse_keyword(src, ptr,
		PARSE_KEYWORD_IMPLICIT_NONE);
	if (i > 0)
	{
		stmt->type = PARSE_STMT_IMPLICIT_NONE;
		return i;
	}

	i = parse_keyword(src, ptr,
		PARSE_KEYWORD_IMPLICIT);
	if (i == 0) return 0;

	stmt->type = PARSE_STMT_IMPLICIT;
	unsigned len;
	stmt->implicit = parse_implicit_list(src, &ptr[i], &len);
	if (!stmt->implicit) return 0;
	i += len;

	return i;
}
