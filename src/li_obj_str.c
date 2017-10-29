#include "li.h"

static void deinit(li_object *obj)
{
    li_string_free(li_to_string(obj));
}

static li_cmp_t compare(li_object *o1, li_object *o2)
{
    return li_string_cmp(li_to_string(o1), li_to_string(o2));
}

static int length(li_object *str)
{
    return li_string_length(li_to_string(str));
}

static li_object *ref(li_object *str, int k)
{
    return li_character(li_string_ref(li_to_string(str), k));
}

static void display(li_object *obj, FILE *fp)
{
    fprintf(fp, "%s", li_string_bytes(li_to_string(obj)));
}

static void write(li_object *obj, FILE *fp)
{
    fprintf(fp, "\"%s\"", li_string_bytes(li_to_string(obj)));
}

const li_type_t li_type_string = {
    .name = "string",
    .deinit = deinit,
    .write = write,
    .display = display,
    .compare = compare,
    .length = length,
    .ref = ref,
};

extern li_object *li_string(li_string_t str)
{
    li_string_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_string);
    obj->string = str;
    return (li_object *)obj;
}

static li_object *p_make_string(li_object *args) {
    li_object *obj;
    char *s;
    int k;
    li_assert_nargs(1, args);
    li_assert_integer(li_car(args));
    k = li_to_integer(li_car(args)) + 1;
    s = li_allocate(li_null, k, sizeof(*s));
    while (k >= 0)
        s[k--] = '\0';
    obj = li_string(li_string_make(s));
    free(s);
    return obj;
}

static li_object *p_string(li_object *args) {
    char *str;
    int i;
    str = li_allocate(li_null, li_length(args)+1, sizeof(char));
    for (i = 0; args; i++, args = li_cdr(args)) {
        if (!li_is_character(li_car(args))) {
            free(str);
            li_error("not a character", li_car(args));
        }
        str[i] = li_to_character(li_car(args));
    }
    str[i] = '\0';
    return li_string(li_string_make(str));
}

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
static li_object *p_is_string(li_object *args) {
    li_assert_nargs(1, args);
    return li_boolean(li_is_string(li_car(args)));
}

static li_object *p_string_append(li_object *args) {
    li_string_t str, tmp;
    if (!args)
        li_error("wrong number of args", args);
    li_assert_string(li_car(args));
    str = li_string_copy(li_to_string(li_car(args)));
    for (args = li_cdr(args); args; args = li_cdr(args)) {
        li_assert_string(li_car(args));
        tmp = str;
        str = li_string_append(str, li_to_string(li_car(args)));
        li_string_free(tmp);
    }
    return li_string(str);
}

static li_object *p_string_to_list(li_object *args) {
    li_object *head, *tail;
    li_string_t str;
    unsigned long i;
    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    str = li_to_string(li_car(args));
    head = tail = li_null;
    for (i = 0; i < li_string_length(str); ++i) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_character(li_string_ref(str, i)),
                        li_null));
        else
            head = tail = li_cons(li_character(li_string_ref(str, i)), li_null);
    }
    return head;
}

static li_object *p_string_to_number(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    return li_number(li_num_with_chars(li_string_bytes(li_to_string(
                        li_car(args)))));
}

static li_object *p_string_to_vector(li_object *args) {
    li_object *head, *tail;
    li_string_t str;
    size_t i;
    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    str = li_to_string(li_car(args));
    head = tail = li_null;
    for (i = 0; i < li_string_length(str); ++i) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_character(li_string_ref(str, i)),
                        li_null));
        else
            head = tail = li_cons(li_character(li_string_ref(str, i)), li_null);
    }
    return li_vector(head);
}

static li_object *p_string_to_symbol(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    return li_symbol(li_string_bytes(li_to_string(li_car(args))));
}

extern void li_define_string_functions(li_environment_t *env)
{
    li_define_primitive_procedure(env, "make-string", p_make_string);
    li_define_primitive_procedure(env, "string", p_string);
    li_define_primitive_procedure(env, "string?", p_is_string);
    li_define_primitive_procedure(env, "string-append", p_string_append);
    li_define_primitive_procedure(env, "string->list", p_string_to_list);
    li_define_primitive_procedure(env, "string->number", p_string_to_number);
    li_define_primitive_procedure(env, "string->symbol", p_string_to_symbol);
    li_define_primitive_procedure(env, "string->vector", p_string_to_vector);
}