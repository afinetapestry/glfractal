//
//  clerr.h
//  edgedetection
//
//  Created by Jonathan Hatchett on 20/02/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __CLERR_H__
#define __CLERR_H__

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include <OpenCL/cl.h>

const char * _clGetErrorMessage(int err);

#ifdef __CPLUSPLUS
}
#endif

#endif
