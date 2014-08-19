#include <stdio.h>
#include "li.h"
#include "proc.h"

#define li_is_tagged_list(exp, tag) (li_is_pair(exp) && car(exp) == li_symbol(tag))

#define li_is_application(exp)      li_is_list(exp)
#define li_is_self_evaluating(exp)  !(li_is_pair(exp) || li_is_symbol(exp))
#define li_is_quoted(exp)           li_is_tagged_list(exp, "quote")
#define li_is_quasiquoted(exp)      li_is_tagged_list(exp, "quasiquote")
#define li_is_unquoted(exp)         li_is_tagged_list(exp, "unquote")
#define li_is_unquoted_splicing(exp) li_is_tagged_list(exp, "unquote-splicing")

#define check_syntax(pred, exp) if (!(pred)) error("eval", "bad syntax", exp);

static li_object *eval_quasiquote(li_object *exp, li_object *env);
static li_object *expand_macro(li_object *mac, li_object *args);
static li_object *extend_environment(li_object *vars, li_object *vals,
        li_object *base_env);
static li_object *list_of_values(li_object *exps, li_object *env);

li_object *apply(li_object *proc, li_object *args) {
    li_object *head, *tail, *obj;

    if (li_is_primitive(proc))
        return li_to_primitive(proc)(args);
    head = tail = li_null;
    while (args) {
        obj = cons(li_symbol("quasiquote"), cons(car(args), li_null));
        if (!tail)
            head = tail = cons(obj, li_null);
        else {
            tail = set_cdr(tail, cons(obj, li_null));
        }
        args = cdr(args);
    }
    return eval(cons(proc, head), li_to_compound(proc).env);
}

li_object *eval(li_object *exp, li_object *env) {
    li_object *seq, *proc, *args;

    while (!li_is_self_evaluating(exp)) {
        if (li_is_symbol(exp)) {
            return li_environment_lookup(env, exp);
        } else if (li_is_quoted(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            return cadr(exp);
        } else if (li_is_quasiquoted(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            return eval_quasiquote(cadr(exp), env);
        } else if (li_is_application(exp)) {
            proc = eval(car(exp), env);
            args = cdr(exp);
            if (li_is_procedure(proc))
                args = list_of_values(args, env);
            if (li_is_compound(proc)) {
                env = extend_environment(li_to_compound(proc).vars, args,
                                         li_to_compound(proc).env);
                for (seq = li_to_compound(proc).body; seq && cdr(seq);
                     seq = cdr(seq))
                    eval(car(seq), env);
                exp = car(seq);
            } else if (li_is_macro(proc)) {
                exp = expand_macro(proc, args);
            } else if (li_is_primitive(proc)) {
                return li_to_primitive(proc)(args);
            } else if (li_is_syntax(proc)) {
                exp = li_to_syntax(proc)(args, env);
            } else {
                error("apply", "not applicable", proc);
            }
        } else {
            error("eval", "unknown expression type", exp);
        }
    }
    return exp;
}

li_object *eval_quasiquote(li_object *exp, li_object *env) {
    li_object *head, *iter, *tail;

    if (!li_is_pair(exp))
        return exp;
    else if (li_is_unquoted(exp))
        return eval(cadr(exp), env);
    else if (li_is_unquoted_splicing(car(exp))) {
        head = tail = li_null;
        for (iter = eval(cadar(exp), env); iter; iter = cdr(iter)) {
            if (head)
                tail = set_cdr(tail, cons(car(iter), li_null));
            else
                head = tail = cons(car(iter), li_null);
        }
        if (tail) {
            set_cdr(tail, eval_quasiquote(cdr(exp), env));
            return head;
        } else {
            return eval_quasiquote(cdr(exp), env);
        }
    }
    return cons(eval_quasiquote(car(exp), env), eval_quasiquote(cdr(exp), env));
}

li_object *expand_macro(li_object *mac, li_object *args) {
    li_object *env, *ret, *seq;

    ret = li_null;
    env = extend_environment(li_to_macro(mac).vars, args, li_to_macro(mac).env);
    for (seq = li_to_macro(mac).body; seq; seq = cdr(seq))
        ret = eval(car(seq), env);
    return ret;
}

li_object *extend_environment(li_object *vars, li_object *vals, li_object *env)
{
    for (env = li_environment(env); vars; vars = cdr(vars), vals = cdr(vals)) {
        if (li_is_symbol(vars)) {
            append_variable(vars, vals, env);
            return env;
        }
        if (!vals)
            break;
        append_variable(car(vars), car(vals), env);
    }
    if (vars || vals)
        error("#[anonymous-procedure]", "wrong number of args", vars);
    return env;
}

li_object *list_of_values(li_object *exps, li_object *env) {
    li_object *head, *node, *tail;

    head = li_null;
    while (exps) {
        tail = cons(eval(car(exps), env), li_null);
        node = head ? set_cdr(node, tail) : (head = tail);
        exps = cdr(exps);
    }
    return head;
}

li_object *setup_environment(void) {
    li_object *env;

    env = li_environment(li_null);
    append_variable(li_symbol("user-initial-environment"), env, env);
    append_variable(li_symbol("null"), li_null, env);
    append_variable(li_boolean(li_true), li_boolean(li_true), env);
    append_variable(li_boolean(li_false), li_boolean(li_false), env);
    define_primitive_procedures(env);
    return env;
}
