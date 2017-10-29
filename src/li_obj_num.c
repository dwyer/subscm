#include "li.h"

#include <math.h>

static void write(li_object *obj, FILE *f)
{
    li_num_t num = li_to_number(obj);
    if (!li_num_is_exact(num))
        fprintf(f, "%f", li_num_to_dec(num));
    else if (li_num_is_integer(num))
        fprintf(f, "%ld", li_num_to_int(num));
    else
        fprintf(f, "%s%ld/%ld",
                li_rat_is_negative(num.real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(num.real.exact)),
                li_nat_to_int(li_rat_den(num.real.exact)));
}

static li_cmp_t compare(li_object *obj1, li_object *obj2)
{
    return li_num_cmp(li_to_number(obj1), li_to_number(obj2));
}

const li_type_t li_type_number = {
    .name = "number",
    .write = write,
    .compare = compare,
};

extern li_object *li_number(li_num_t n)
{
    li_num_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_number);
    obj->number = n;
    return (li_object *)obj;
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
    return li_boolean(li_num_is_complex(li_to_number(obj)));
}

static li_object *p_is_real(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_num_is_real(li_to_number(obj)));
}

static li_object *p_is_rational(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_num_is_rational(li_to_number(obj)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
static li_object *p_is_integer(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_integer(li_car(args)));
}

static li_object *p_is_exact(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_num_is_exact(li_to_number(obj)));
}

static li_object *p_is_inexact(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_boolean(!li_num_is_exact(x));
}

static li_object *p_is_zero(li_object *args) {
    li_num_t num;
    li_parse_args(args, "n", &num);
    return li_boolean(li_num_is_zero(num));
}

static li_object *p_is_positive(li_object *args) {
    li_num_t num;
    li_parse_args(args, "n", &num);
    return li_boolean(!li_num_is_negative(num));
}

static li_object *p_is_negative(li_object *args) {
    li_num_t num;
    li_parse_args(args, "n", &num);
    return li_boolean(li_num_is_negative(num));
}

static li_object *p_is_odd(li_object *args) {
    li_int_t x;
    li_parse_args(args, "i", &x);
    return li_boolean(x % 2 != 0);
}

static li_object *p_is_even(li_object *args) {
    li_int_t x;
    li_parse_args(args, "i", &x);
    return li_boolean(x % 2 == 0);
}

static li_object *p_max(li_object *args) {
    li_num_t max;
    if (!args)
        li_error("wrong number of args", args);
    max = li_to_number(li_car(args));
    while ((args = li_cdr(args)))
        max = li_num_max(max, li_to_number(li_car(args)));
    return li_number(max);
}

static li_object *p_min(li_object *args) {
    li_num_t min;
    if (!args)
        li_error("wrong number of args", args);
    min = li_to_number(li_car(args));
    while ((args = li_cdr(args)))
        min = li_num_min(min, li_to_number(li_car(args)));
    return li_number(min);
}

static li_object *p_add(li_object *args) {
    li_num_t result;
    if (!args)
        return li_number(li_num_with_int(0));
    li_assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    while (args) {
        li_assert_number(li_car(args));
        result = li_num_add(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_sub(li_object *args) {
    li_num_t result;
    if (!args)
        li_error("wrong number of args", args);
    li_assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    if (!args)
        result = li_num_neg(result);
    while (args) {
        li_assert_number(li_car(args));
        result = li_num_sub(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_div(li_object *args) {
    li_num_t result;
    if (!args)
        li_error("wrong number of args", args);
    li_assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    if (!args)
        result = li_num_div(li_num_with_int(1), result);
    while (args) {
        li_assert_number(li_car(args));
        result = li_num_div(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_floor_div(li_object *args) {
    li_num_t x, y;
    li_parse_args(args, "nn", &x, &y);
    return li_number(li_num_floor(li_num_div(x, y)));
}

static li_object *p_abs(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_abs(x));
}

static li_object *p_quotient(li_object *args) {
    li_int_t x, y;
    li_parse_args(args, "ii", &x, &y);
    if (y == 0)
        li_error_f("arg2 must be non-zero");
    return li_number(li_num_with_int(x / y));
}

static li_object *p_mul(li_object *args) {
    li_num_t result;
    if (!args)
        return li_number(li_num_with_int(1));
    li_assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    while (args) {
        li_assert_number(li_car(args));
        result = li_num_mul(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_remainder(li_object *args) {
    li_int_t x, y;
    li_parse_args(args, "ii", &x, &y);
    if (y == 0)
        li_error_f("arg2 must be non-zero");
    return li_number(li_num_with_int(x % y));
}

static li_object *p_modulo(li_object *args) {
    li_int_t x, y, z;
    li_parse_args(args, "ii", &x, &y);
    if (y == 0)
        li_error_f("arg2 must be non-zero");
    z = x % y;
    if (z * y < 0)
        z += y;
    return li_number(li_num_with_int(z));
}

static li_object *p_gcd(li_object *args) {
    int a, b, c;
    if (!args)
        return li_number(li_num_with_int(0));
    li_assert_integer(li_car(args));
    a = labs(li_to_integer(li_car(args)));
    while ((args = li_cdr(args))) {
        li_assert_integer(li_car(args));
        b = labs(li_to_integer(li_car(args)));
        while (b) {
            c = b;
            b = a % b;
            a = c;
        }
    }
    return li_number(li_num_with_int(a));
}

static li_object *p_floor(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_floor(x));
}

static li_object *p_ceiling(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_ceiling(x));
}

static li_object *p_truncate(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_truncate(x));
}

static li_object *p_round(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_round(x));
}

static li_object *p_exp(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_exp(x));
}

static li_object *p_log(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_log(x));
}

static li_object *p_sin(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_sin(x));
}

static li_object *p_cos(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_cos(x));
}

static li_object *p_tan(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_tan(x));
}

static li_object *p_asin(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_asin(x));
}

static li_object *p_acos(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_acos(x));
}

static li_object *p_atan(li_object *args) {
    li_num_t x, y;
    if (li_cdr(args)) {
        li_parse_args(args, "nn", &x, &y);
        return li_number(li_num_atan2(y, x));
    } else {
        li_parse_args(args, "n", &x);
        return li_number(li_num_atan(x));
    }
}

static li_object *p_sqrt(li_object *args) {
    li_num_t x;
    li_parse_args(args, "n", &x);
    return li_number(li_num_sqrt(x));
}

static li_object *p_expt(li_object *args) {
    li_num_t x, y;
    li_parse_args(args, "nn", &x, &y);
    return li_number(li_num_expt(x, y));
}

static li_object *p_number_to_string(li_object *args) {
    static char buf[BUFSIZ];
    li_assert_nargs(1, args);
    li_assert_number(li_car(args));
    li_num_to_chars(li_to_number(li_car(args)), buf, sizeof(buf));
    return li_string(li_string_make(buf));
}

extern void li_define_number_functions(li_environment_t *env)
{
    /* Numerical operations */
    li_define_primitive_procedure(env, "number?", p_is_number);
    li_define_primitive_procedure(env, "complex?", p_is_complex);
    li_define_primitive_procedure(env, "real?", p_is_real);
    li_define_primitive_procedure(env, "rational?", p_is_rational);
    li_define_primitive_procedure(env, "integer?", p_is_integer);
    li_define_primitive_procedure(env, "exact?", p_is_exact);
    li_define_primitive_procedure(env, "inexact?", p_is_inexact);
    li_define_primitive_procedure(env, "zero?", p_is_zero);
    li_define_primitive_procedure(env, "positive?", p_is_positive);
    li_define_primitive_procedure(env, "negative?", p_is_negative);
    li_define_primitive_procedure(env, "odd?", p_is_odd);
    li_define_primitive_procedure(env, "even?", p_is_even);
    li_define_primitive_procedure(env, "max", p_max);
    li_define_primitive_procedure(env, "min", p_min);
    li_define_primitive_procedure(env, "+", p_add);
    li_define_primitive_procedure(env, "*", p_mul);
    li_define_primitive_procedure(env, "-", p_sub);
    li_define_primitive_procedure(env, "/", p_div);
    li_define_primitive_procedure(env, "//", p_floor_div);
    li_define_primitive_procedure(env, "abs", p_abs);
    li_define_primitive_procedure(env, "quotient", p_quotient);
    li_define_primitive_procedure(env, "remainder", p_remainder);
    li_define_primitive_procedure(env, "modulo", p_modulo);
    li_define_primitive_procedure(env, "gcd", p_gcd);
    li_define_primitive_procedure(env, "floor", p_floor);
    li_define_primitive_procedure(env, "ceiling", p_ceiling);
    li_define_primitive_procedure(env, "truncate", p_truncate);
    li_define_primitive_procedure(env, "round", p_round);
    li_define_primitive_procedure(env, "exp", p_exp);
    li_define_primitive_procedure(env, "log", p_log);
    li_define_primitive_procedure(env, "sin", p_sin);
    li_define_primitive_procedure(env, "cos", p_cos);
    li_define_primitive_procedure(env, "tan", p_tan);
    li_define_primitive_procedure(env, "asin", p_asin);
    li_define_primitive_procedure(env, "acos", p_acos);
    li_define_primitive_procedure(env, "atan", p_atan);
    li_define_primitive_procedure(env, "sqrt", p_sqrt);
    li_define_primitive_procedure(env, "expt", p_expt);
    li_define_primitive_procedure(env, "number->string", p_number_to_string);
}