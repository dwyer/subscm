#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "proc.h"

#define is_tagged_list(exp, tag)    (is_pair(exp) && car(exp) == symbol(tag))

#define is_and(exp)                 is_tagged_list(exp, "and")
#define is_application(exp)         is_list(exp)
#define is_assert(exp)              is_tagged_list(exp, "assert")
#define is_assignment(exp)          is_tagged_list(exp, "set!")
#define is_begin(exp)               is_tagged_list(exp, "begin")
#define is_cond(exp)                is_tagged_list(exp, "cond")
#define is_cond_else_clause(exp)    is_tagged_list(exp, "else")
#define is_definition(exp)          is_tagged_list(exp, "define")
#define is_delay(exp)               is_tagged_list(exp, "delay")
#define is_if(exp)                  is_tagged_list(exp, "if")
#define is_lambda(exp)              is_tagged_list(exp, "lambda")
#define is_load(exp)                is_tagged_list(exp, "load")
#define is_let(exp)                 is_tagged_list(exp, "let")
#define is_or(exp)                  is_tagged_list(exp, "or")
#define is_self_evaluating(exp)     (exp && !is_pair(exp) && !is_symbol(exp))
#define is_quoted(exp)              is_tagged_list(exp, "quote")
#define is_variable(exp)            is_symbol(exp)

#define make_begin(seq)             cons(symbol("begin"), seq)
#define make_if(pred, con, alt)     cons(symbol("if"), \
                                         cons(pred, cons(con, cons(alt, nil))))
#define make_lambda(p, b)           cons(symbol("lambda"), cons(p, b))

object *apply(object *procedure, object *arguments);
object *eval_assert(object *exp, object *env);
object *eval_assignment(object *exp, object *env);
object *eval_definition(object *exp, object *env);
object *eval_let(object *exp, object *env);
object *eval_load(object *exp, object *env);
object *eval_sequence(object *exps, object *env);
object *extend_environment(object *vars, object *vals, object *base_env);
object *list_of_values(object *exps, object *env);
object *lookup_variable_value(object *exp, object *env);
object *set_variable_value(object *var, object *val, object *env);

object *apply(object *proc, object *args) {
    if (is_primitive(proc))
        return to_primitive(proc)(args);
    return eval(cons(proc, args), to_compound(proc).env);
}

object *define_variable(object *var, object *val, object *env) {
    int i;

    for (i = 0; i < env->data.env.size; i++)
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return var;
        }
    if (env->data.env.size == env->data.env.cap) {
        env->data.env.cap *= 2;
        env->data.env.array = realloc(env->data.env.array, env->data.env.cap *
                                      sizeof(*env->data.env.array));
    }
    env->data.env.array[env->data.env.size].var = var;
    env->data.env.array[env->data.env.size].val = val;
    env->data.env.size++;
    return var;
}

object *eval(object *exp, object *env) {
    object *seq;

    while (!is_self_evaluating(exp))
        if (is_variable(exp))
            return lookup_variable_value(exp, env);
        else {
            for (seq = exp; seq; seq = cdr(seq))
                if (seq && !is_pair(seq))
                    error("eval", "ill-formed special form", exp);
            if (is_quoted(exp)) {
                return cadr(exp);
            } else if (is_delay(exp)) {
                return compound(cons(nil, cdr(exp)), env);
            } else if (is_lambda(exp)) {
                return compound(cdr(exp), env);
            } else if (is_definition(exp)) {
                return eval_definition(exp, env);
            } else if (is_assignment(exp)) {
                return eval_assignment(exp, env);
            } else if (is_if(exp)) {
                if (is_true(eval(cadr(exp), env)))
                    exp = caddr(exp);
                else if (cdddr(exp))
                    exp = cadddr(exp);
                else
                    return boolean(false);
            } else if (is_cond(exp)) {
                while ((exp = cdr(exp))) {
                    if (is_tagged_list(car(exp), "else") ||
                        is_true(eval(caar(exp), env))) {
                        for (exp = cdar(exp); cdr(exp); exp = cdr(exp))
                            eval(car(exp), env);
                        break;
                    }
                }
                if (!exp)
                    return boolean(false);
                exp = car(exp);
            } else if (is_begin(exp)) {
                if (!cdr(exp))
                    return nil;
                while (cdr(exp = cdr(exp)))
                    eval(car(exp), env);
                exp = car(exp);
            } else if (is_and(exp)) {
                if (!cdr(exp))
                    return boolean(true);
                while (cdr(exp = cdr(exp)))
                    if (is_false(eval(car(exp), env)))
                        return boolean(false);
                exp = car(exp);
            } else if (is_or(exp)) {
                object *obj;

                if (!cdr(exp))
                    return boolean(false);
                while (cdr(exp = cdr(exp)))
                    if (is_true(obj = eval(car(exp), env)))
                        return obj;
                exp = car(exp);
            } else if (is_let(exp)) {
                return eval_let(exp, env);
            } else if (is_assert(exp)) {
                return eval_assert(exp, env);
            } else if (is_load(exp)) {
                return eval_load(cdr(exp), env);
            } else if (is_application(exp)) {
                object *proc = eval(car(exp), env);
                object *args = list_of_values(cdr(exp), env);
                if (is_compound(proc)) {
                    exp = to_compound(proc).proc;
                    env = to_compound(proc).env;
                    env = extend_environment(car(exp), args, env);
                    while (cdr(exp = cdr(exp)))
                        eval(car(exp), env);
                    exp = car(exp);
                } else if (is_primitive(proc)) {
                    return to_primitive(proc)(args);
                } else {
                    error("apply", "not applicable", proc);
                }
            } else {
                error("eval", "unknown expression type", exp);
            }
        }
    return exp;
}

object *eval_assignment(object *exp, object *env) {
    if (length(exp) != 3 || !is_symbol(cadr(exp)))
        error("set!", "ill-formed special form", exp);
    return set_variable_value(cadr(exp), eval(caddr(exp), env), env);
}

object *eval_assert(object *exp, object *env) {
    object *ret;

    if (!cdr(exp) || cddr(exp))
        error("assert", "ill-formed special form", exp);
    ret = eval(cadr(exp), env);
    if (is_false(ret))
        error("assert", "assertion violated", exp);
    return nil;
}

object *eval_definition(object *exp, object *env) {
    int k;

    if ((k = length(exp)) == 3 && is_symbol(cadr(exp)))
        return define_variable(cadr(exp), eval(caddr(exp), env), env);
    else if (k >= 3 && is_pair(cadr(exp)))
        return define_variable(caadr(exp),
                               eval(make_lambda(cdadr(exp), cddr(exp)), env),
                               env);
    error("define", "ill-formed special form", exp);
    return nil;
}

object *eval_let(object *exp, object *env) {
    object *bind;
    object *vars;
    object *vals;

    if (length(exp) < 3 || length(cadr(exp)) == -1)
        error("let", "ill-formed special form", exp);
    bind = cadr(exp);
    vars = nil;
    vals = nil;
    while (bind) {
        vars = cons(caar(bind), vars);
        vals = cons(eval(cadar(bind), env), vals);
        bind = cdr(bind);
    }
    return eval_sequence(cddr(exp), extend_environment(vars, vals, env));
}

object *eval_load(object *exp, object *env) {
    if (!exp || cdr(exp))
        error("load", "wrong number of args", exp);
    if (!is_string(car(exp)))
        error("load", "arg must be a string", exp);
    load(to_string(car(exp)), env);
    return nil;
}

object *eval_sequence(object *exps, object *env) {
    object *val;

    while (exps) {
        val = eval(car(exps), env);
        exps = cdr(exps);
    }
    return val;
}

object *extend_environment(object *vars, object *vals, object *env) {
    env = environment(env);
    while (vars) {
        if (is_symbol(vars)) {
            define_variable(vars, vals, env);
            return env;
        }
        if (!vals)
            break;
        define_variable(car(vars), car(vals), env);
        vars = cdr(vars);
        vals = cdr(vals);
    }
    if (vars || vals)
        error("#[anonymous procedure]", "wrong number of args", vars);
    return env;
}

object *list_of_values(object *exps, object *env) {
    object *head, *node, *tail;

    head = nil;
    while (exps) {
        tail = cons(eval(car(exps), env), nil);
        node = head ? set_cdr(node, tail) : (head = tail);
        exps = cdr(exps);
    }
    return head;
}

object *lookup_variable_value(object *var, object *env) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var)
                return env->data.env.array[i].val;
        env = env->data.env.base;
    }
    error("eval", "unbound variable", var);
    return nil;
}

object *set_variable_value(object *var, object *val, object *env) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var) {
                env->data.env.array[i].val = val;
                return var;
            }
        env = env->data.env.base;
    }
    error("eval", "unbound variable", var);
    return nil;
}

object *setup_environment(void) {
    object *env;

    env = environment(nil);
    define_variable(symbol("user-initial-environment"), env, env);
    define_variable(boolean(true), boolean(true), env);
    define_variable(boolean(false), boolean(false), env);
    define_primitive_procedures(env);
    return env;
}
