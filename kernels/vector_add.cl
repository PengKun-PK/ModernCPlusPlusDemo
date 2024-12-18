__kernel void vectorAdd(__global const float* a,
                        __global const float* b,
                        __global float* c)
{
    int i = get_global_id(0);
    c[i] = a[i] + b[i];
}

__kernel void vectorAddEx(
    __global const float* a,
    __global const float* b,
    __global float* c
)
{
    __local float a_cache[256];  // 局部内存缓存
    __local float b_cache[256];

    int gid = get_global_id(0);  // 全局ID
    int lid = get_local_id(0);   // 局部ID

    // 加载数据到局部内存
    a_cache[lid] = a[gid];
    b_cache[lid] = b[gid];

    // 同步确保数据加载完成
    barrier(CLK_LOCAL_MEM_FENCE);

    // 计算
    c[gid] = a_cache[lid] + b_cache[lid];
}
