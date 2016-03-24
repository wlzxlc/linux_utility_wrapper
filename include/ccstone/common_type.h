/*
 *  Copyright (c) 2014 The linux_utility_wrapper project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_
#include <stdint.h>
typedef int status_t;

/////////////////////////////////////////////////////////
//                                                     //
// |<--- 31 ---->|<-30-----------16|<-15-------0->|    //
//    signed bit      custom bit        line bit       //
//        1bit           15bit         16bit = 65536   //  
//                                                     //
/////////////////////////////////////////////////////////

#define CSM_XSTR(X) #X
#define CSM_STR(X) CSM_XSTR(X)

//Get number of line from a status
#define CSM_LINE(X) ((X) & 0xFFFF)
//Get value of custom field from a status
#define CSM_CUSTOM(X) (((X) >> 16) & 0x7FFF)
//Construct a status with custom(15bit) and number of line(16bit)
#define CSM_CONSTRUCK_STATUS(custom_15bit,line_16bit) ((1 << 31) | (custom_15bit << 16)|(line_16bit & 0xFFFF))
//Construct a status with custom(15bit)
#define CSM_STATUS(status15bit) ((CSM_CONSTRUCK_STATUS(status15bit, __LINE__ )))
//Genarate a specify mask, it's contain X '1'.
#define CSM_BITS_MASK(X) (~((~0) << (X)))
//Get value of begin to end bit widh field
#define CSM_BITS_VALUES(begin,end,v) (((v) >> (begin)) & CSM_BITS_MASK((end) - (begin) + 1))

#endif //end of COMMON_TYPE_H

