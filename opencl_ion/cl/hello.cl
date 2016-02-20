__kernel void hello(__global int *pts){
    const int x = get_global_id(0);

    pts[x] = x;
}
