; ModuleID = 'tests/loop1.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

define i32 @main() nounwind {
entry:
  %retval = alloca i32, align 4
  %q = alloca i32, align 4
  %r = alloca i32, align 4
  %i = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 2, i32* %q, align 4
  store i32 5, i32* %r, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32* %r, align 4
  %mul = mul nsw i32 7, %1
  %add = add nsw i32 %mul, 22
  store i32 %add, i32* %x, align 4
  %2 = load i32* %q, align 4
  %3 = load i32* %i, align 4
  %mul1 = mul nsw i32 5, %3
  %add2 = add nsw i32 %2, %mul1
  %4 = load i32* %x, align 4
  %mul3 = mul nsw i32 3, %4
  %add4 = add nsw i32 %add2, %mul3
  %add5 = add nsw i32 %add4, 5
  store i32 %add5, i32* %q, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32* %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %6 = load i32* %q, align 4
  %sub = sub nsw i32 %6, 44
  ret i32 %sub
}
