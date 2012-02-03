#include <assert.h>
#include <stdio.h>
#include "object.h"
#include "output.h"

void write_object(object *obj, FILE *f, int h);
void write_pair(object *obj, FILE *f, int h);
void write_vector(object *obj, FILE *f, int h);

void write(object *obj, FILE *f) {
    write_object(obj, f, 0);
}

void display(object *obj, FILE *f) {
    write_object(obj, f, 1);
}

void write_object(object *obj, FILE *f, int h) {
    if (is_null(obj))
        fprintf(f, "()");
    else if (is_locked(obj))
        fprintf(f, "...");
    else if (is_number(obj))
        fprintf(f, "%g", to_number(obj));
    else if (is_string(obj) && h)
        fprintf(f, "%s", to_string(obj));
    else if (is_string(obj))
        fprintf(f, "\"%s\"", to_string(obj));
    else if (is_symbol(obj))
        fprintf(f, "%s", to_symbol(obj));
    else if (is_primitive(obj))
        fprintf(f, "#[primitive-procedure]");
    else if (is_compound(obj))
        fprintf(f, "#[compound-procedure]");
    else if (is_pair(obj))
        write_pair(obj, f, h);
    else if (is_vector(obj))
        write_vector(obj, f, h);
}

void write_pair(object *obj, FILE *f, int h) {
    object *iter;

    obj->locked = 1;
    iter = obj;
    fprintf(f, "(");
    do {
        write_object(car(iter), f, h);
        iter = cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (is_pair(iter));
    if (iter) {
        fprintf(f, ". ");
        write_object(iter, f, h);
    }
    fprintf(f, ")");
    obj->locked = 0;
}

void write_vector(object *obj, FILE *f, int h) {
    int k;

    fprintf(f, "#(");
    for (k = 0; k < vector_length(obj); k++) {
        write_object(vector_ref(obj, k), f, h);
        if (k < vector_length(obj) - 1)
            fprintf(f, " ");
    }
    fprintf(f, ")");
}
