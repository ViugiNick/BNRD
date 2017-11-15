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

struct set_specifc_data {
    int pos;
    int set;
    int prev; /* 1: set, 2: unset, 0: not found */
};

static int
my_line_trace_specify(int line, rb_event_flag_t *events_ptr, void *ptr)
{
    struct set_specifc_data *data = (struct set_specifc_data *)ptr;

    if (data->pos == line) {
        data->prev = *events_ptr & RUBY_EVENT_SPECIFIED_LINE ? 1 : 2;
        if (data->set) {
            *events_ptr = *events_ptr | RUBY_EVENT_SPECIFIED_LINE;
        }
        else {
            *events_ptr = *events_ptr & ~RUBY_EVENT_SPECIFIED_LINE;
        }
        return 0; /* found */
    }
    else {
	    return 1;
    }
}

VALUE
my_rb_iseqw_line_trace_specify(VALUE iseqval, int needed_line, VALUE set)
{
    struct set_specifc_data data;

    data.prev = 0;
    data.pos = needed_line;
    if (data.pos < 0) rb_raise(rb_eTypeError, "`pos' is negative");

    switch (set) {
      case Qtrue:  data.set = 1; break;
      case Qfalse: data.set = 0; break;
      default:
	    rb_raise(rb_eTypeError, "`set' should be true/false");
    }

    rb_iseqw_line_trace_each(iseqval, my_line_trace_specify, (void *)&data);

//    if (data.prev == 0) {
//	    rb_raise(rb_eTypeError, "`pos' is out of range.");
//    }
    return data.prev == 1 ? Qtrue : Qfalse;
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

    int cnt = 0;

    my_rb_iseqw_line_trace_specify(rb_iseqw_new(iseq), lineno, Qtrue);

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
    fprintf(stderr, "add_breakpoint\n");
    c_add_breakpoint(FIX2UINT(lineno), rb_iseqw_to_iseq(file_iseq));
}