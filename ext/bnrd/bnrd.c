#include "bnrd.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>


VALUE mBNRD = Qnil;

void Init_bnrd();

static void add_breakpoint(VALUE self, VALUE lineno, VALUE rb_iseq);

void Init_bnrd() {
    mBNRD = rb_define_module("BNRD");
    rb_define_module_function(mBNRD, "add_breakpoint", add_breakpoint, 2);
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

static void
c_add_breakpoint(unsigned int lineno, rb_iseq_t *iseq)
{
    int i;
    VALUE *code;
    VALUE child = rb_ary_tmp_new(3);
    unsigned int size;
    size_t n;
    VALUE str = rb_str_new(0, 0);

    size = iseq->body->line_info_size;
    const struct iseq_line_info_entry *table = iseq->body->line_info_table;

    int left = 0;
    int right = size;

    if (iseq->body->catch_table) {
        left += 1;
        right -= 2;
    }

    int cnt = 0;

    for (i = left; i < right; i++, cnt++) {
        //debug_print(iseq_inspect(iseq));
        //fprintf(stderr, "%d:: table[i].position(%d) table[i].line_no(%d) == lineno(%d)\n", i, table[i].position, table[i].line_no, lineno);
        if(table[i].line_no == lineno) {
            //fprintf(stderr, "table[%d].position(%d)\n", i, table[i].position);
            //debug_print(UINT2NUM(cnt));
            rb_iseqw_line_trace_specify(rb_iseqw_new(iseq), UINT2NUM(cnt), Qtrue);
        }
    }

    size = iseq->body->iseq_size;
    code = rb_iseq_original_iseq(iseq);
    for (n = 0; n < size;) {
        n += rb_iseq_disasm_insn(str, code, n, iseq, child);
    }

    for (i = 0; i < RARRAY_LEN(child); i++) {
        VALUE isv = rb_ary_entry(child, i);
        rb_iseq_t *tmp_iseq = rb_iseq_check((rb_iseq_t *)isv);
        c_add_breakpoint(lineno, tmp_iseq);
    }
}

static void
add_breakpoint(VALUE self, VALUE lineno, VALUE file_iseq)
{
    c_add_breakpoint(FIX2UINT(lineno), iseqw_check(file_iseq));
}