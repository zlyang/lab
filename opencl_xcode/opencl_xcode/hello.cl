__kernel void hello(__global int *restrict data){
    const int x = get_global_id(0);
    data[x] = x;
}
