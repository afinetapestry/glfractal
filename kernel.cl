#ifndef M_PI
#	define M_PI 3.14159265359f
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#ifdef cl_khr_fp64
#	define fptype double
#	define fptype4 double4
#else
#	define fptype float
#	define fptype4 float4
#endif

#define unroll10(s) {{s} {s} {s} {s} {s} {s} {s} {s} {s} {s}}
#define unroll100(s) {unroll10(unroll10(s))}
#define forunroll10(a, b, c, s) {for (a; b; ) {unroll10(s; c;)}}
#define forunroll100(a, b, c, s) {for (a; b; ) {unroll100(s; c;)}}

__kernel void fractal(write_only image2d_t output, fptype left, fptype top, fptype right, fptype bottom, unsigned int iterations, unsigned int w, unsigned int h) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	fptype4 color = (fptype4)(0.0f, 0.0f, 0.0f, 1.0f);

	fptype zr = 0.0f;
	fptype zi = 0.0f;
	fptype cr = left + coord.x * (right - left) / w;
	fptype ci = top + coord.y * (bottom - top) / h;
	fptype rsquared = 0.0f;
	fptype isquared = 0.0f;

	forunroll10 (unsigned int i = 0, i < iterations, ++i, {
	//#pragma unroll 100
	//for (unsigned int i = 0; i < iterations; ++i) {
		zi = zr * zi;
		zi += zi;
		zi += ci;

		zr = rsquared - isquared + cr;

		rsquared = zr * zr;
		isquared = zi * zi;

		if (rsquared + isquared > 4.0f) {
			color.x = sin(((fptype)i / (fptype)(iterations)) * M_PI / 2.0f);
			//color.x = (fptype)i / ((fptype)iterations / 100.0f);
			break;
		}
	})

	write_imagef(output, coord, color);
}

