#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

typedef half half4 __attribute__((ext_vector_type(4)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef double double4 __attribute__((ext_vector_type(4)));

void kernel testFloatTypes() {
  /// "isAtomic" : false
  half scalarHalf;
  /// "isAtomic" : false
  half4 vectorHalf;
  /// "isAtomic" : false
  float scalarFloat;
  /// "isAtomic" : false
  float4 vectorFloat;
  /// "isAtomic" : false
  double scalarDouble;
  /// "isAtomic" : false
  double4 vectorDouble;
}

/// "isVirtual" : false
void kernel testFlatImageTys(image1d_t img1d, image2d_t img2d, image3d_t img3d) {
}

/// "isVirtual" : false
void kernel testImageArrays(image1d_array_t img1d, image2d_array_t img2d, image2d_array_depth_t img2da,
                            image2d_array_msaa_t img2dmsaa, image2d_array_msaa_depth_t img2damd) {
}

/// "isVirtual" : false
void kernel testOtherImageTys(image1d_buffer_t img1d, image2d_depth_t img2ddepth,
                              image2d_msaa_t msaa, image2d_msaa_depth_t img2ddepthmsaa) {
}

/// "isVirtual" : false
void kernel testOpaqueTys(sampler_t samplerTy) {
    event_t eventTy;

#if  __OPENCL_C_VERSION__ >= 200
    queue_t queueTy;
    ndrange_t ndrangeTy;
    clk_event_t clkeventTy;
    reserve_id_t reservedIDTy;
#endif
}

#if  __OPENCL_C_VERSION__ >= 200
void kernel testAtomicTys() {
    /// "isAtomic" : true
    atomic_int ai;
    /// "isAtomic" : true
    atomic_uint aui;
    /// "isAtomic" : true
    atomic_long al;
    /// "isAtomic" : true
    atomic_ulong aul;
    /// "isAtomic" : true
    atomic_float af;
    /// "isAtomic" : true
    atomic_double ad;
    /// "isAtomic" : true
    atomic_intptr_t ap;
    /// "isAtomic" : true
    atomic_uintptr_t au;
    /// "isAtomic" : true
    atomic_size_t as;
    /// "isAtomic" : true
    atomic_ptrdiff_t apdt;
}
#endif
