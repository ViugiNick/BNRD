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
add_breakpoint(VALUE self, VALUE path, VALUE lineno, VALUE rb_iseq)
{
    return rb_iseqw_line_trace_specify(rb_iseq, lineno, Qtrue);
}