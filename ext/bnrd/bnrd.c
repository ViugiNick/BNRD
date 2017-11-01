#include "bnrd.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>


VALUE mBNRD = Qnil;

void Init_bnrd();

static VALUE add_breakpoint(VALUE self, VALUE path, VALUE lineno, VALUE iseq);

void Init_bnrd() {
    mBNRD = rb_define_module("BNRD");
    rb_define_module_function(mBNRD, "add_breakpoint", add_breakpoint, 2);
}

static VALUE
add_breakpoint(VALUE self, VALUE path, VALUE lineno, VALUE iseq)
{
    return rb_iseqw_line_trace_specify(iseq, lineno, Qtrue);
}