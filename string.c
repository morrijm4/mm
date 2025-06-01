#pragma once

#include <assert.h>
#include <string.h>

int ends_with(const char *str, const char* suffix)
{
    if (str == NULL || suffix == NULL) {
	return 0;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
	return 0;
    }

    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

#ifdef UNIT_TESTS

int main() {
    assert(ends_with("index.html", ".html"));
    assert(!ends_with("index.html", ".css"));
    assert(!ends_with("a", "abc"));
    assert(!ends_with(NULL, NULL));
}

#endif
