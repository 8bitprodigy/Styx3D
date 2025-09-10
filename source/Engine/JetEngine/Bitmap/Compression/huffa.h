/****************************************************************************************/
/*  HUFFA.H                                                                             */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef CRB_HUFFA_H
#define CRB_HUFFA_H

/** 

	HuffArray:

	calls O0Huff and O1Huff and uses best of two

	returns JE_TRUE = success
				JE_FALSE = failure
	CompressFlag = JE_TRUE to pack , JE_FALSE to unpack
**/

extern jeBoolean HuffArray(uint8 *RawArray,uint32 RawFileLen,
					 uint8 *HuffArray,uint32 * HuffArrayLenPtr,int Type);

#define HUFFA_TYPE_BEST -1
#define HUFFA_TYPE_DEC  0 /* 0 for backwards compatibility */
#define HUFFA_TYPE_O1   1
#define HUFFA_TYPE_O0   2
#define HUFFA_TYPE_O0NB 3 /* no blocking */
#define HUFFA_TYPE_NONE 4

/** you should always use HuffArray with TYPE setting, instead of
the below.  They are provided as APIs just for completeness **/

extern jeBoolean O0HuffArray(uint8 *RawArray,uint32 RawFileLen,
					 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag);

extern jeBoolean O0HuffArrayNoBlock(uint8 *RawArray,uint32 RawFileLen,
					 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag);

extern jeBoolean O1HuffArray(uint8 *RawArray,uint32 RawFileLen,
					 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag);

extern jeBoolean O0HuffArrayBII_RT(uint8 *rawArray,uint32 rawLen,struct LBitIOInfo * BII,jeBoolean cFlag);
extern jeBoolean O0HuffArrayBII(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag);
extern jeBoolean O0HuffArrayBII_block(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag);
extern jeBoolean O0HuffArrayBII_noblock(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag);

#endif

