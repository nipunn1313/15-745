; ModuleID = 'loop1-m2r.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

define i32 @main() nounwind {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %q.0 = phi i32 [ 2, %entry ], [ %add5, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %mul = mul nsw i32 7, 5
  %add = add nsw i32 %mul, 22
  %mul1 = mul nsw i32 5, %i.0
  %add2 = add nsw i32 %q.0, %mul1
  %mul3 = mul nsw i32 3, %add
  %add4 = add nsw i32 %add2, %mul3
  %add5 = add nsw i32 %add4, 5
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %sub = sub nsw i32 %q.0, 44
  ret i32 %sub
}
