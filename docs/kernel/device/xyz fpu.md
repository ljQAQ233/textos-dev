# overview

> i387 fpu

- <https://wiki.osdev.org/FPU>
- <https://qiita.com/Egh2Deywos/items/13a22f014b5f71d06c88>

# native

> 8.7.1 Native Mode
> 
> The native mode for handling floating-point exceptions is selected by setting CR0.NE[bit 5] to 1. In this mode, if the
> x87 FPU detects an exception condition while executing a floating-point instruction and the exception is unmasked
> (the mask bit for the exception is cleared), the x87 FPU sets the flag for the exception and the ES flag in the x87
> FPU status word. It then invokes the software exception handler through the floating-point-error exception (#MF,
> exception vector 16), immediately before execution of any of the following instructions in the processor’s instruction stream:
> • The next floating-point instruction, unless it is one of the non-waiting instructions (FNINIT, FNCLEX, FNSTSW, FNSTCW, FNSTENV, and FNSAVE).
> • The next WAIT/FWAIT instruction.
> • The next MMX instruction.
> If the next floating-point instruction in the instruction stream is a non-waiting instruction, the x87 FPU
> executes the instruction without invoking the software exception handler.

# fpu 状态

