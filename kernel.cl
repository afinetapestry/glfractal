#ifndef M_PI
#	define M_PI 3.14159265359f
#endif

#ifdef cl_khr_fp64
#	pragma OPENCL EXTENSION cl_khr_fp64 : enable
#	define fptype double
#	define fptype4 double4
#else
#	define fptype float
#	define fptype4 float4
#endif

__kernel void fractal(write_only image2d_t output, fptype left, fptype top, fptype right, fptype bottom, int iterations) {
	const int2 size = get_image_dim(output);

	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	int i;
	fptype4 color = (fptype4)(0.0f, 0.0f, 0.0f, 1.0f);

	fptype zr = 0.0f;
	fptype zi = 0.0f;
	fptype cr = left + coord.x * (right - left) / size.x;
	fptype ci = top + coord.y * (bottom - top) / size.y;
	fptype rsquared = 0.0f;
	fptype isquared = 0.0f;

	for (i = 0; rsquared + isquared <= 4.0f && i < iterations; i++) {
		zi = zr * zi;
		zi += zi;
		zi += ci;

		zr = rsquared - isquared + cr;

		rsquared = zr * zr;
		isquared = zi * zi;

		if (rsquared + isquared > 4.0f) {
			color.x = sin(((fptype)i / (fptype)iterations) * M_PI / 2.0f);
			//color.x = (fptype)i / (fptype)iterations;
		}
	}

	write_imagef(output, coord, color);
}

