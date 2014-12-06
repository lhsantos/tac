.table
int a[3]={0,0,0}
int b[3]={1,1,1}
int c[3]={0,0,0}
.code
//print loop
mov $0,3
mov $1,0
L2:
mov $2, &c
mov $2, $2[$1]
print $2
add $1, $1, 1
sub $3, $0, $1
brnz L2, $3
