// Quicksort algorithm, as proposed by Cormen et. al
.table
int size
.code
// swaps two array elements
swap:
mov $0, #0[#1]
mov $1, #0[#2]
mov #0[#1], $1
mov #0[#2], $0
return

//partition(A, p, r)
//     x = A[r]
//     i = p - 1
//     for j = p to r - 1
//         if A[i] <= x
//             i = i + 1
//             swap(A, i, j)
//     swap(A, i + 1, r)
//     return i + 1
partition:
mov $0, #0[#2]
sub $1, #1, 1
sub $3, #2, 1
P0:
sleq $2, #1, $3
brz P1, $2
mov $2, #0[#1]
sleq $2, $2, $0
brz P2, $2
add $1, $1, 1
param #0
param $1
param #1
call swap, 3
P2:
add #1, #1, 1
jump P0
P1:
param #0
add $1, $1, 1
param $1
param #2
call swap, 3
return $1

//quicksort(A, p, r):
//  if p < r:
//    q := partition(A, p, r)
//    quicksort(A, p, q - 1)
//    quicksort(A, q + 1, r)
quick:
slt $0, #1, #2
brz Q1, $0
param #0
param #1
param #2
call partition, 3
pop $0
param #0
param #1
sub $1, $0, 1
param $1
call quick, 3
param #0
add $1, $0, 1
param $1
param #2
call quick, 3
Q1:
return

main:
// read size
println '?'
scani size
mema $3, size
// $0 = 0, $1 = size - 1
mov $0, 0
sub $1, size, 1
// while $0 < size
L1:
slt $2, $0, size
brz L2, $2
// v[$0] = rand
rand $2
mod $2, $2, 100
mov $3[$0], $2
// print $2
print $2
// if $0 < size - 1, print ", "
slt $2, $0, $1
brz L3, $2
print ','
print ' '
L3:
add $0, $0, 1
jump L1 // loop
L2:
println
// call quick(&v, 0, size - 1)
param $3
param 0
sub $0, size, 1
param $0
call quick, 3
// $0 = 0, $1 = size - 1
mov $0, 0
sub $1, size, 1
// while $0 < size
L4:
slt $2, $0, size
brz L5, $2
// print v[$0]
mov $2, $3[$0]
print $2
// if $0 < size - 1, print ", "
slt $2, $0, $1
brz L6, $2
print ','
print ' '
L6:
add $0, $0, 1
jump L4 // loop
L5:
println
memf $3
