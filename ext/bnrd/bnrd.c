#include "bnrd.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>


VALUE mBNRD = Qnil;

void Init_bnrd();

static VALUE add_breakpoint(VALUE self, VALUE path, VALUE lineno, VALUE rb_iseq);

void Init_bnrd() {
    mBNRD = rb_define_module("BNRD");
    rb_define_module_function(mBNRD, "add_breakpoint", add_breakpoint, 3);
}

static void debug_print(VALUE v) {
    ID sym_puts = rb_intern("puts");
    ID sym_inspect = rb_intern("inspect");
    rb_funcall(rb_mKernel, sym_puts, 1,
        rb_funcall(v, sym_inspect, 0));
}

static VALUE
iseq_inspect(const rb_iseq_t *iseq)
{
    if (!iseq->body->location.label) {
	return rb_sprintf("#<ISeq: uninitialized>");
    }
    else {
	return rb_sprintf("#<ISeq:%s@%s>", RSTRING_PTR(iseq->body->location.label), RSTRING_PTR(rb_iseq_path(iseq)));
    }
}

static const rb_iseq_t *
iseqw_check(VALUE iseqw)
{
    rb_iseq_t *iseq = DATA_PTR(iseqw);

    if (!iseq->body) {
	    ibf_load_iseq_complete(iseq);
    }

    if (!iseq->body->location.label) {
	    rb_raise(rb_eTypeError, "uninitialized InstructionSequence");
    }
    return iseq;
}

static VALUE
c_add_breakpoint(VALUE path, unsigned int lineno, rb_iseq_t *iseq)
{
    int i;
    VALUE *code;
    VALUE child = rb_ary_tmp_new(3);
    unsigned int size;
    size_t n;
    VALUE str = rb_str_new(0, 0);

    size = iseq->body->iseq_size;
    code = rb_iseq_original_iseq(iseq);
    for (n = 0; n < size;) {
        n += rb_iseq_disasm_insn(str, code, n, iseq, child);
    }
    rb_iseq_t *next_iseq = NULL;

    for (i = 0; i < RARRAY_LEN(child); i++) {
        VALUE isv = rb_ary_entry(child, i);
        rb_iseq_t *tmp_iseq = rb_iseq_check((rb_iseq_t *)isv);

        unsigned int tmp_iseq_starts = rb_iseq_line_no(tmp_iseq, 0);
        if(tmp_iseq_starts < lineno)
        {
            next_iseq = tmp_iseq;
        }
    }

    if(next_iseq == NULL) {
        unsigned int iseq_starts = rb_iseq_line_no(iseq, 0);
        return rb_iseqw_line_trace_specify(rb_iseqw_new(iseq), UINT2NUM(lineno - iseq_starts - 1), Qtrue);
    }

    return c_add_breakpoint(path, lineno, next_iseq);
}

static VALUE
add_breakpoint(VALUE self, VALUE path, VALUE lineno, VALUE file_iseq)
{
    return c_add_breakpoint(path, FIX2UINT(lineno), iseqw_check(file_iseq));
}