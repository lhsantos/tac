// Quicksort algorithm, as proposed by Cormen et. al
.table
int v[20] = {20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}
int size = 20

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
// call quick(&v, 0, size - 1)
mov $0, &v
param $0
param 0
sub $0, size, 1
param $0
call quick, 3
// $0 = 0, $1 = size - 1
mov $0, 0
sub $1, size, 1
// while $0 < size
L1:
slt $2, $0, size
brz L2, $2
// print v[$0]
mov $2, &v
mov $2, $2[$0]
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
