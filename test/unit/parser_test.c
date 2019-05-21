#include <runtime.h>
#include <stdlib.h>
#include <string.h>

#include <buffer.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define COUNT_ELM   10

#define test_assert(expr) do { \
if (expr) ; else { \
    msg_err("%s -- failed at %s:%d\n", #expr, __FILE__, __LINE__); \
    return false; \
} \
} while (0)

#define test_strings_equal(s1, s2) do { \
if (strcmp(s1, s2) != 0) { \
    msg_err("\"%s\" != \"%s\" -- failed at %s:%d\n", s1, s2, __FILE__, __LINE__); \
    return false; \
} \
} while (0)

#define test_no_errors() do { \
    if (errors_count) { \
        msg_err("%d parse error(s), last: %b\n", errors_count, last_error); \
    } \
    test_assert(errors_count == 0); \
} while (0)

tuple root;
CLOSURE_1_1(finish, void, heap, void*);
void finish(heap h, void *v)
{
    root = v;
}

int errors_count = 0;
string last_error;

parser p;

CLOSURE_0_1(perr, void, string);
void perr(string s)
{
    errors_count++;
    last_error = s; // TODO: copy string here
}

void parse_tuple_string(heap h, char *str)
{
    root = NULL;
    errors_count = 0;
    last_error = NULL;

    buffer b = wrap_buffer_cstring(h, str);
    parser_feed(p, b);
    /* deallocate_buffer(b); */
}

#define PARSE_TEST(name, str) \
    boolean _check_##name(heap h); \
\
    boolean name(heap h) \
    { \
        parse_tuple_string(h, str); \
\
        return !_check_##name(h); \
    } \
\
    boolean _check_##name(heap h)


PARSE_TEST(empty_string_test, "")
{
    test_no_errors();
    test_assert(root == NULL);
    return true;
}


PARSE_TEST(empty_tuple_test, "()")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 0);
    test_no_errors();
    return true;
}

PARSE_TEST(empty_tuple_with_whitespaces_test, "  (   )    ")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 0);
    return true;
}

PARSE_TEST(empty_vector_test, "[]")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 0);
    return true;
}

PARSE_TEST(all_is_comment_test, "#[]")
{
    test_no_errors();
    test_assert(root == NULL);
    return true;
}

PARSE_TEST(partial_comment_test, "#[]\n()")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 0);
    return true;
}

PARSE_TEST(tuple_simple_test, "(key:value)")
{
    test_no_errors();

    test_assert(root != NULL);
    test_assert(root->count == 1);

    buffer v1 = table_find(root, intern(wrap_buffer_cstring(h, "key")));
    test_assert(v1 != NULL);
    test_strings_equal(v1->contents, "value");
    deallocate_buffer(v1);

    return true;
}

PARSE_TEST(quoted_value_test, "(key:\"value\")")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 1);
    buffer buf = allocate_buffer(h, 128);
    bprintf(buf, "%t", root);
    test_strings_equal(buf->contents, "(key:value)");
    deallocate_buffer(buf);

    return true;
}

PARSE_TEST(unknown_terminal_test, "(key:value()")
{
    test_assert(errors_count == 1);
    test_strings_equal(last_error->contents, "unknown property discriminator 40");

    return true;
}

PARSE_TEST(vector_simple_test, "[val1]")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 1);

    buffer v1 = table_find(root, intern_u64(0));
    test_assert(v1 != NULL);
    test_strings_equal(v1->contents, "val1");

    return true;
}

PARSE_TEST(tuple_2elements_test, "(key1:value1 key2:value2)")
{
    test_no_errors();

    test_assert(root != NULL);
    test_assert(root->count == 2);

    buffer v1 = table_find(root, intern(wrap_buffer_cstring(h, "key1")));
    test_assert(v1 != NULL);
    test_strings_equal(v1->contents, "value1");
    deallocate_buffer(v1);

    buffer v2 = table_find(root, intern(wrap_buffer_cstring(h, "key2")));
    test_assert(v2 != NULL);
    test_strings_equal(v2->contents, "value2");
    deallocate_buffer(v2);

    return true;
}

PARSE_TEST(vector_2elements_test, "[val1 val2]")
{
    test_no_errors();
    test_assert(root != NULL);
    test_assert(root->count == 2);

    buffer v1 = table_find(root, intern_u64(0));
    test_assert(v1 != NULL);
    test_strings_equal(v1->contents, "val1");

    buffer v2 = table_find(root, intern_u64(1));
    test_assert(v2 != NULL);
    test_strings_equal(v2->contents, "val2");

    return true;
}

void init (heap h)
{
    p = tuple_parser(h, closure(h, finish, h), closure(h, perr));
}

typedef boolean (*test_func)(heap h);

test_func TESTS[] = {
    empty_string_test,
    empty_tuple_test,
    empty_tuple_with_whitespaces_test,
    empty_vector_test,
    all_is_comment_test,
    partial_comment_test,
    tuple_simple_test,
    quoted_value_test,
    unknown_terminal_test,
    vector_simple_test,
    tuple_2elements_test,
    vector_2elements_test,
    NULL
};

int main(int argc, char **argv)
{
    heap h = init_process_runtime();
    init(h);

    boolean failure = false;

    for (int i = 0; TESTS[i]; ++i) {
        failure |= TESTS[i](h);
    }

    if (failure) {
        msg_err("Test failed\n"); exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
