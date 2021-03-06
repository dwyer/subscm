#include "li.h"
#include "li_lib.h"
#include "li_num.h"

#include <math.h>

#define are_exact(x, y) ((x)->exact && (y)->exact)

#define CACHE_SIZE 256

static li_num_t *cache;
static li_num_t *li_zero;
static li_num_t *li_one;

union part {
    li_rat_t exact;
    li_dec_t inexact;
};

struct li_num_t {
    LI_OBJ_HEAD;
    const li_bool_t exact;
    const union part real;
};

static void write(li_num_t *num, li_port_t *port)
{
    if (!li_num_is_exact(num))
        li_port_printf(port, "%f", li_num_to_dec(num));
    else if (li_num_is_integer(num))
        li_port_printf(port, "%d", li_num_to_int(num));
    else
        li_port_printf(port, "%s%ld/%ld",
                li_rat_is_negative(num->real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(num->real.exact)),
                li_nat_to_int(li_rat_den(num->real.exact)));
}

const li_type_t li_type_number = {
    .name = "number",
    .size = sizeof(li_num_t),
    .write = (li_write_f *)write,
    .compare = (li_cmp_f *)li_num_cmp,
};

static li_num_t *make_num(li_bool_t exactness, li_rat_t *exact, li_dec_t *inexact)
{
    li_num_t *n = (li_num_t *)li_create(&li_type_number);
    *(li_bool_t *)&n->exact = exactness;
    if (exact)
        (*(union part *)&n->real).exact = *exact;
    else
        (*(union part *)&n->real).inexact = *inexact;
    return n;
}

static li_num_t *make_exact(li_rat_t exact)
{
    if (!exact.neg && exact.num.data < CACHE_SIZE && exact.den.data == 1)
        return &cache[exact.num.data];
    return make_num(LI_TRUE, &exact, NULL);
}

static li_num_t *make_inexact(li_dec_t inexact)
{
    return make_num(LI_FALSE, NULL, &inexact);
}

extern li_bool_t li_num_is_integer(li_num_t *x)
{
    if (li_num_is_exact(x))
        return !li_rat_is_integer(x->real.exact);
    return x->real.inexact == floor(x->real.inexact);
}

extern li_cmp_t li_num_cmp(li_num_t *x, li_num_t *y)
{
    static const li_dec_t epsilon = 1.0 / (1 << 22);
    li_dec_t z;
    if (are_exact(x, y))
        return li_rat_cmp(x->real.exact, y->real.exact);
    z = li_num_to_dec(x) - li_num_to_dec(y);
    if (fabs(z) < epsilon)
        return LI_CMP_EQ;
    return z < 0 ?  LI_CMP_LT : LI_CMP_GT;
}

static li_num_t *li_num_exact_to_inexact(li_num_t *x)
{
    if (x->exact)
        return make_inexact(li_rat_to_dec(x->real.exact));
    return x;
}

extern li_num_t *li_num_max(li_num_t *x, li_num_t *y)
{
    li_bool_t exact = are_exact(x, y);
    if (li_num_cmp(x, y) == LI_CMP_LT)
        x = y;
    if (!exact && x->exact)
        return li_num_exact_to_inexact(x);
    return x;
}

extern li_num_t *li_num_min(li_num_t *x, li_num_t *y)
{
    li_bool_t exact = are_exact(x, y);
    if (li_num_cmp(x, y) == LI_CMP_GT)
        x = y;
    if (!exact && x->exact)
        return li_num_exact_to_inexact(x);
    return x;
}

extern li_num_t *li_num_with_dec(li_dec_t x)
{
    return make_inexact(x);
}

extern size_t li_num_to_chars(li_num_t *x, char *s, size_t n)
{
    if (!li_num_is_exact(x))
        return snprintf(s, n, "%f", li_num_to_dec(x));
    else if (li_num_is_integer(x))
        return snprintf(s, n, "%d", li_num_to_int(x));
    else
        return snprintf(s, n, "%s%ld/%ld",
                li_rat_is_negative(x->real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(x->real.exact)),
                li_nat_to_int(li_rat_den(x->real.exact)));
}

extern li_num_t *li_num_with_int(int x)
{
    return make_exact(li_rat_with_int(x));
}

extern li_num_t *li_num_with_rat(li_rat_t x)
{
    return make_exact(x);
}

extern li_num_t *li_num_with_chars(const char *s, int radix)
{
    li_dec_t x;
    if (radix != 10)
        li_error_fmt("only radix of 10 is supported");
    x = li_dec_parse(s);
    return x == floor(x) ? li_num_with_int(x) : li_num_with_dec(x);
}

extern int li_num_to_int(li_num_t *x)
{
    if (li_num_is_exact(x))
        return li_rat_to_int(x->real.exact);
    return (li_int_t)x->real.inexact;
}

extern li_dec_t li_num_to_dec(li_num_t *x)
{
    if (li_num_is_exact(x))
        return li_rat_to_dec(x->real.exact);
    return x->real.inexact;
}

extern li_num_t *li_num_add(li_num_t *x, li_num_t *y)
{
    if (are_exact(x, y))
        return make_exact(li_rat_add(x->real.exact, y->real.exact));
    return make_inexact(li_num_to_dec(x) + li_num_to_dec(y));
}

extern li_num_t *li_num_sub(li_num_t *x, li_num_t *y)
{
    if (are_exact(x, y))
        return make_exact(li_rat_sub(x->real.exact, y->real.exact));
    return make_inexact(li_num_to_dec(x) - li_num_to_dec(y));
}

extern li_num_t *li_num_mul(li_num_t *x, li_num_t *y)
{
    if (are_exact(x, y))
        return make_exact(li_rat_mul(x->real.exact, y->real.exact));
    return make_inexact(li_num_to_dec(x) * li_num_to_dec(y));
}

extern li_num_t *li_num_div(li_num_t *x, li_num_t *y)
{
    if (are_exact(x, y))
        return make_exact(li_rat_div(x->real.exact, y->real.exact));
    return make_inexact(li_num_to_dec(x) / li_num_to_dec(y));
}

extern li_num_t *li_num_neg(li_num_t *x)
{
    if (li_num_is_exact(x))
        return make_exact(li_rat_neg(x->real.exact));
    return make_inexact(-x->real.inexact);
}

/*
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
static li_object *p_is_number(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj));
}

static li_object *p_is_complex(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_num_is_complex((li_num_t *)obj));
}

static li_object *p_is_real(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_num_is_real((li_num_t *)obj));
}

static li_object *p_is_rational(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_num_is_rational((li_num_t *)obj));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
static li_object *p_is_integer(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_is_integer(li_car(args)));
}

static li_object *p_is_exact(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return li_boolean(li_num_is_exact(x));
}

static li_object *p_is_inexact(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return li_boolean(!li_num_is_exact(x));
}

static li_object *p_is_zero(li_object *args) {
    li_num_t *num;
    li_parse_args(args, "n", &num);
    return li_boolean(li_num_is_zero(num));
}

static li_object *p_is_positive(li_object *args) {
    li_num_t *num;
    li_parse_args(args, "n", &num);
    return li_boolean(!li_num_is_negative(num));
}

static li_object *p_is_negative(li_object *args) {
    li_num_t *num;
    li_parse_args(args, "n", &num);
    return li_boolean(li_num_is_negative(num));
}

static li_object *p_is_odd(li_object *args) {
    li_int_t x;
    li_parse_args(args, "I", &x);
    return li_boolean(x % 2 != 0);
}

static li_object *p_is_even(li_object *args) {
    li_int_t x;
    li_parse_args(args, "I", &x);
    return li_boolean(x % 2 == 0);
}

static li_object *p_max(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn.", &x, &y, &args);
    x = li_num_max(x, y);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_max(x, y);
    }
    return (li_object *)x;
}

static li_object *p_min(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn.", &x, &y, &args);
    x = li_num_min(x, y);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_min(x, y);
    }
    return (li_object *)x;
}

static li_object *p_add(li_object *args) {
    li_num_t *x, *y;
    if (!args)
        return (li_object *)li_zero;
    li_parse_args(args, "n.", &x, &args);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_add(x, y);
    }
    return (li_object *)x;
}

static li_object *p_sub(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "n.", &x, &args);
    if (!args)
        return (li_object *)li_num_neg(x);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_sub(x, y);
    }
    return (li_object *)x;
}

static li_object *p_mul(li_object *args) {
    li_num_t *x, *y;
    if (!args)
        return (li_object *)li_one;
    li_parse_args(args, "n.", &x, &args);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_mul(x, y);
    }
    return (li_object *)x;
}

static li_object *p_div(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "n.", &x, &args);
    if (!args)
        x = li_num_div(li_one, x);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_div(x, y);
    }
    return (li_object *)x;
}

static li_object *p_floor_div(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn", &x, &y);
    return (li_object *)li_num_floor(li_num_div(x, y));
}

static li_object *p_abs(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_abs(x);
}

static li_object *p_quotient(li_object *args) {
    li_int_t x, y;
    li_parse_args(args, "II", &x, &y);
    if (y == 0)
        li_error_fmt("arg2 must be non-zero");
    return (li_object *)li_num_with_int(x / y);
}

static li_object *p_remainder(li_object *args) {
    li_int_t x, y;
    li_parse_args(args, "II", &x, &y);
    if (y == 0)
        li_error_fmt("arg2 must be non-zero");
    return (li_object *)li_num_with_int(x % y);
}

static li_object *p_modulo(li_object *args) {
    li_int_t x, y, z;
    li_parse_args(args, "II", &x, &y);
    if (y == 0)
        li_error_fmt("arg2 must be non-zero");
    z = x % y;
    if (z * y < 0)
        z += y;
    return (li_object *)li_num_with_int(z);
}

/* TODO: extern this */
static li_int_t li_int_gcd(li_int_t x, li_int_t y)
{
    while (y) {
        li_int_t z = y;
        y = x % y;
        x = z;
    }
    return labs(x);
}

/* TODO: extern this */
static li_int_t li_int_lcm(li_int_t a, li_int_t b)
{
    return labs(a * b) / li_int_gcd(a, b);
}

static li_object *p_gcd(li_object *args) {
    li_int_t a, b; /* TODO: support li_num_t */
    if (!args)
        return (li_object *)li_zero;
    li_parse_args(args, "I.", &a, &args);
    while (args) {
        li_parse_args(args, "I.", &b, &args);
        a = li_int_gcd(a, b);
    }
    return (li_object *)li_num_with_int(a);
}

static li_object *p_lcm(li_object *args) {
    li_int_t a, b; /* TODO: support li_num_t */
    if (!args)
        return (li_object *)li_one;
    li_parse_args(args, "I.", &a, &args);
    while (args) {
        li_parse_args(args, "I.", &b, &args);
        a = li_int_lcm(a, b);
    }
    return (li_object *)li_num_with_int(a);
}

static li_object *p_numerator(li_object *args) {
    li_num_t *q;
    li_parse_args(args, "n", &q);
    if (!q->exact)
        li_error_fmt("not exact: ~a", args); /* TODO: support inexact numbers */
    return (li_object *)make_exact(li_rat_with_nat(q->real.exact.num));
}

static li_object *p_denominator(li_object *args) {
    li_num_t *q;
    li_parse_args(args, "n", &q);
    if (!q->exact)
        li_error_fmt("not exact: ~a", args); /* TODO: support inexact numbers */
    return (li_object *)make_exact(li_rat_with_nat(q->real.exact.den));
}

static li_object *p_floor(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_floor(x);
}

static li_object *p_ceiling(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_ceiling(x);
}

static li_object *p_truncate(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_truncate(x);
}

static li_object *p_round(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_round(x);
}

static li_object *p_exp(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_exp(x);
}

static li_object *p_log(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_log(x);
}

static li_object *p_sin(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_sin(x);
}

static li_object *p_cos(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_cos(x);
}

static li_object *p_tan(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_tan(x);
}

static li_object *p_asin(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_asin(x);
}

static li_object *p_acos(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_acos(x);
}

static li_object *p_atan(li_object *args) {
    li_num_t *x, *y;
    if (li_cdr(args)) {
        li_parse_args(args, "nn", &x, &y);
        return (li_object *)li_num_atan2(y, x);
    } else {
        li_parse_args(args, "n", &x);
        return (li_object *)li_num_atan(x);
    }
}

static li_object *p_square(li_object *args)
{
    li_num_t *z;
    li_parse_args(args, "n", &z);
    return (li_object *)li_num_mul(z, z);
}

static li_object *p_sqrt(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_sqrt(x);
}

static li_object *p_expt(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn", &x, &y);
    return (li_object *)li_num_expt(x, y);
}

static li_object *p_inexact(li_object *args) {
    li_num_t *z;
    li_parse_args(args, "n", &z);
    return (li_object *)li_num_exact_to_inexact(z);
}

static li_object *p_number_to_string(li_object *args) {
    static char buf[BUFSIZ];
    li_num_t *z;
    li_parse_args(args, "n", &z);
    li_num_to_chars(z, buf, sizeof(buf));
    return (li_object *)li_string_make(buf);
}

static li_object *p_string_to_number(li_object *args) {
    li_str_t *str;
    int radix = 10;
    if (li_length(args) == 1)
        li_parse_args(args, "s", &str);
    else
        li_parse_args(args, "si", &str, &radix);
    return (li_object *)li_num_with_chars(li_string_bytes(str), radix);
}

extern void li_define_number_functions(li_env_t *env)
{
    /* Numerical operations */
    lilib_defproc(env, "number?", p_is_number);
    lilib_defproc(env, "complex?", p_is_complex);
    lilib_defproc(env, "real?", p_is_real);
    lilib_defproc(env, "rational?", p_is_rational);
    lilib_defproc(env, "integer?", p_is_integer);
    lilib_defproc(env, "exact?", p_is_exact);
    lilib_defproc(env, "inexact?", p_is_inexact);
    /* lilib_defproc(env, "finite?", p_is_finite); */
    /* lilib_defproc(env, "infinite?", p_is_infinite); */
    /* lilib_defproc(env, "nan?", p_is_nan); */
    lilib_defproc(env, "zero?", p_is_zero);
    lilib_defproc(env, "positive?", p_is_positive);
    lilib_defproc(env, "negative?", p_is_negative);
    lilib_defproc(env, "odd?", p_is_odd);
    lilib_defproc(env, "even?", p_is_even);
    lilib_defproc(env, "max", p_max);
    lilib_defproc(env, "min", p_min);
    lilib_defproc(env, "+", p_add);
    lilib_defproc(env, "*", p_mul);
    lilib_defproc(env, "-", p_sub);
    lilib_defproc(env, "/", p_div);
    lilib_defproc(env, "//", p_floor_div);
    lilib_defproc(env, "abs", p_abs);
    lilib_defproc(env, "quotient", p_quotient);
    lilib_defproc(env, "remainder", p_remainder);
    lilib_defproc(env, "modulo", p_modulo);
    lilib_defproc(env, "gcd", p_gcd);
    lilib_defproc(env, "lcm", p_lcm);
    lilib_defproc(env, "numerator", p_numerator);
    lilib_defproc(env, "denominator", p_denominator);
    lilib_defproc(env, "floor", p_floor);
    lilib_defproc(env, "ceiling", p_ceiling);
    lilib_defproc(env, "truncate", p_truncate);
    lilib_defproc(env, "round", p_round);
    lilib_defproc(env, "exp", p_exp);
    lilib_defproc(env, "log", p_log);
    lilib_defproc(env, "sin", p_sin);
    lilib_defproc(env, "cos", p_cos);
    lilib_defproc(env, "tan", p_tan);
    lilib_defproc(env, "asin", p_asin);
    lilib_defproc(env, "acos", p_acos);
    lilib_defproc(env, "atan", p_atan);
    lilib_defproc(env, "square", p_square);
    lilib_defproc(env, "sqrt", p_sqrt);
    lilib_defproc(env, "expt", p_expt);
    /* lilib_defproc(env, "exact", p_exact); */
    lilib_defproc(env, "inexact", p_inexact);
    lilib_defproc(env, "number->string", p_number_to_string);
    lilib_defproc(env, "string->number", p_string_to_number);
}

static li_num_t _cache[CACHE_SIZE] = {
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=0}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=1}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=2}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=3}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=4}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=5}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=6}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=7}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=8}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=9}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=10}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=11}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=12}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=13}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=14}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=15}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=16}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=17}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=18}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=19}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=20}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=21}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=22}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=23}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=24}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=25}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=26}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=27}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=28}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=29}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=30}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=31}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=32}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=33}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=34}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=35}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=36}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=37}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=38}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=39}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=40}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=41}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=42}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=43}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=44}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=45}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=46}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=47}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=48}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=49}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=50}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=51}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=52}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=53}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=54}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=55}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=56}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=57}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=58}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=59}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=60}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=61}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=62}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=63}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=64}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=65}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=66}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=67}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=68}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=69}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=70}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=71}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=72}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=73}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=74}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=75}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=76}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=77}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=78}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=79}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=80}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=81}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=82}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=83}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=84}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=85}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=86}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=87}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=88}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=89}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=90}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=91}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=92}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=93}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=94}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=95}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=96}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=97}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=98}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=99}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=100}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=101}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=102}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=103}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=104}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=105}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=106}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=107}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=108}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=109}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=110}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=111}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=112}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=113}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=114}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=115}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=116}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=117}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=118}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=119}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=120}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=121}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=122}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=123}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=124}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=125}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=126}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=127}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=128}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=129}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=130}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=131}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=132}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=133}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=134}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=135}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=136}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=137}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=138}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=139}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=140}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=141}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=142}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=143}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=144}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=145}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=146}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=147}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=148}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=149}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=150}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=151}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=152}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=153}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=154}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=155}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=156}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=157}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=158}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=159}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=160}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=161}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=162}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=163}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=164}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=165}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=166}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=167}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=168}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=169}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=170}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=171}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=172}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=173}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=174}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=175}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=176}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=177}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=178}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=179}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=180}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=181}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=182}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=183}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=184}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=185}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=186}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=187}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=188}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=189}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=190}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=191}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=192}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=193}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=194}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=195}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=196}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=197}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=198}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=199}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=200}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=201}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=202}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=203}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=204}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=205}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=206}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=207}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=208}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=209}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=210}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=211}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=212}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=213}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=214}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=215}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=216}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=217}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=218}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=219}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=220}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=221}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=222}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=223}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=224}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=225}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=226}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=227}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=228}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=229}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=230}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=231}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=232}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=233}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=234}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=235}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=236}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=237}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=238}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=239}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=240}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=241}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=242}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=243}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=244}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=245}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=246}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=247}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=248}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=249}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=250}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=251}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=252}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=253}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=254}, .den={.data=1}}}},
    {.type=&li_type_number, .exact=1, .real={.exact={.num={.data=255}, .den={.data=1}}}},
};

static li_num_t *cache = _cache;
static li_num_t *li_zero = &_cache[0];
static li_num_t *li_one = &_cache[1];
