#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef half half4 __attribute__((ext_vector_type(4)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef double double4 __attribute__((ext_vector_type(4)));

void kernel testFloatTypes() {
  half scalarHalf;
  half4 vectorHalf;
  float scalarFloat;
  float4 vectorFloat;
  double scalarDouble;
  double4 vectorDouble;
}
