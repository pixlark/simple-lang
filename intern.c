#include "intern.h"

Str_Intern * str_interns;

const char * str_intern_range(const char * start, const char * end)
{
	size_t len = end - start;
	for (int i = 0; i < sb_count(str_interns); i++) {
		if (str_interns[i].len == len &&
			strncmp(str_interns[i].str, start, len) == 0) {
			return str_interns[i].str;
		}
	}
	char * interned = malloc(len + 1);
	strncpy(interned, start, len);
	interned[len] = '\0';
	Str_Intern new_intern = {len, interned};
	sb_push(str_interns, new_intern);
	return new_intern.str;
}

const char * str_intern(const char * str)
{
	return str_intern_range(str, str + strlen(str));
}

void str_intern_test()
{
	const char a[] = "asdf";
	const char b[] = "asdf";
	assert(a != b);
	assert(str_intern(a) == str_intern(b));
	const char c[] = "ASDF";
	assert(str_intern(a) != str_intern(c));
}
