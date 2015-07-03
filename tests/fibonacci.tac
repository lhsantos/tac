.table
int n = 10
.code
fibonacci:
nop
fibonacci2:
fibonacci3:
slt $0, #0, 1 // if first parameter is less than 1, set $0 to 1, otherwise to 0
brz L1, $0 // if $0 is zero, goto L1
return 0
L1:
slt $0, #0, 2 // if first parameter is less than 2, set $0 to 1, otherwise to 0
brz L2, $0 // if $0 is zero, goto L2
return 1
L2:
// call fibonacci with value (#0 - 1)
sub $0, #0, 1 
param $0
call fibonacci, 1
pop $0 // get result on $0
// call fibonacci with value (#0 - 2)
sub $1, #0, 2
param $1
call fibonacci, 1
pop $1 // get result on $1
// adds $0 and $1 and returns
add $0, $0, $1
return $0
main:
nop
param n
call fibonacci, 1
pop $0
println $0
