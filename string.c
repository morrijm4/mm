#include <string.h>

/**
 * @param file_path - Should be valid C string.
 * @param file_path_max_length - max length of file path.
 *
 * @return Pointer to the start of the file extension after the '.' It returns
 * NULL if no extension is found.
 */
char *get_file_extension(char *file_path, size_t file_path_max_length) {
    if (file_path == NULL || file_path_max_length == 0) {
	return NULL;
    }

    size_t file_path_length = strnlen(file_path, file_path_max_length);

    if (file_path_length == file_path_max_length) {
	return NULL;
    }

    if (file_path[file_path_length] == '.') {
	return NULL;
    }

    // Iterate backwards to find '.'
    for (int i = file_path_length; i > 0; --i) {
	if (file_path[i] == '.') {
	    return file_path + i + 1;
	}

	if (file_path[i] == '/') {
	    return NULL;
	}
    }

    return NULL;
}


#ifdef UNIT_TESTS
#include <assert.h>

int main() {
    // Happy path
    assert(strcmp(get_file_extension("index.html", 11), "html") == 0);

    // String length is max length
    assert(get_file_extension("index.html", 10) == NULL);

    // Max length is less than string
    assert(get_file_extension("index.html", 9) == NULL);

    // No extension
    assert(get_file_extension("index", 11) == NULL);

    // End early because '/' was found first
    assert(get_file_extension("api/my.data/web", 100) == NULL);

    // `file_path` is NULL
    assert(get_file_extension(NULL, 100) == NULL);

    // `file_path_max_length` is 0
    assert(get_file_extension("mystring", 0) == NULL);
}

#endif // UNIT_TESTS
