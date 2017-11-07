#include "li.h"

#include <string.h>

struct li_str_t {
    LI_OBJ_HEAD;
    char *bytes;
};

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

static void display(li_str_t *str, li_port_t *port)
{
    li_port_printf(port, "%s", li_string_bytes(str));
}

static void write(li_str_t *str, li_port_t *port)
{
    li_port_printf(port, "\"%s\"", li_string_bytes(str));
}

const li_type_t li_type_string = {
    .name = "string",
    .deinit = deinit,
    .write = (li_write_f *)write,
    .display = (li_write_f *)display,
    .compare = compare,
    .length = length,
    .ref = ref,
};

extern li_str_t *li_string_make(const char *s)
{
    li_str_t *str = li_allocate(NULL, 1, sizeof(*str));
    li_object_init((li_object *)str, &li_type_string);
    str->bytes = strdup(s);
    return str;
}

extern li_str_t *li_string_copy(li_str_t *str)
{
    return li_string_make(str->bytes);
}

extern void li_string_free(li_str_t *str)
{
    free(str->bytes);
}

extern char *li_string_bytes(li_str_t *str)
{
    return str->bytes;
}

extern li_character_t li_string_ref(li_str_t *str, int idx)
{
    li_character_t c;
    const char *s = str->bytes;
    while (idx >= 0) {
        s += li_chr_decode(&c, s);
        idx--;
    }
    return c;
}

extern int li_string_length(li_str_t *str)
{
    return li_chr_count(str->bytes);
}

extern li_cmp_t li_string_cmp(li_str_t *st1, li_str_t *st2)
{
    int res = strcmp(st1->bytes, st2->bytes);
    if (res < 0)
        return LI_CMP_LT;
    if (res > 0)
        return LI_CMP_GT;
    return LI_CMP_EQ;
}

extern li_str_t *li_string_append(li_str_t *str1, li_str_t *str2)
{
    int n1 = strlen(str1->bytes);
    int n2 = strlen(str2->bytes);
    char *s = li_allocate(NULL, n1+n2+1, sizeof(*s));
    int i;
    for (i = 0; i < n1; ++i)
        s[i] = str1->bytes[i];
    for (i = 0; i < n2; ++i)
        s[n1+i] = str2->bytes[i];
    s[n1+n2] = '\0';
    str1 = li_string_make(s);
    free(s);
    return str1;
}
static li_object *p_make_string(li_object *args) {
    li_str_t *str;
    char *bytes;
    int k;
    li_parse_args(args, "i", &k);
    k++;
    bytes = li_allocate(NULL, k, sizeof(*bytes));
    while (k >= 0)
        bytes[k--] = '\0';
    str = li_string_make(bytes);
    free(bytes);
    return (li_object *)str;
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
    return (li_object *)li_string_make(str);
}

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
static li_object *p_is_string(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_string(obj));
}

static li_object *p_string_append(li_object *args) {
    li_str_t *str;
    li_parse_args(args, "s.", &str, &args);
    str = li_string_copy(str);
    for (; args; ) {
        li_str_t *old = str, *end;
        li_parse_args(args, "s.", &end, &args);
        str = li_string_append(str, end);
        li_string_free(old);
    }
    return (li_object *)str;
}

static li_object *p_string_to_list(li_object *args) {
    li_object *head = NULL, *tail = NULL;
    li_str_t *str;
    int i;
    li_parse_args(args, "s", &str);
    for (i = 0; i < li_string_length(str); ++i) {
        li_object *node = li_cons(li_character(li_string_ref(str, i)), NULL);
        if (head)
            tail = li_set_cdr(tail, node);
        else
            head = tail = node;
    }
    return head;
}

static li_object *p_string_to_vector(li_object *args) {
    li_object *head = NULL, *tail = NULL;
    li_str_t *str;
    int i;
    li_parse_args(args, "s", &str);
    for (i = 0; i < li_string_length(str); ++i) {
        li_object *node = li_cons(li_character(li_string_ref(str, i)), li_null);
        if (head)
            tail = li_set_cdr(tail, node);
        else
            head = tail = node;
    }
    return li_vector(head);
}

static li_object *p_string_to_symbol(li_object *args) {
    li_str_t *str;
    li_parse_args(args, "s", &str);
    return (li_object *)li_symbol(li_string_bytes(str));
}

extern void li_define_string_functions(li_env_t *env)
{
    li_define_primitive_procedure(env, "make-string", p_make_string);
    li_define_primitive_procedure(env, "string", p_string);
    li_define_primitive_procedure(env, "string?", p_is_string);
    li_define_primitive_procedure(env, "string-append", p_string_append);
    li_define_primitive_procedure(env, "string->list", p_string_to_list);
    li_define_primitive_procedure(env, "string->symbol", p_string_to_symbol);
    li_define_primitive_procedure(env, "string->vector", p_string_to_vector);
}
