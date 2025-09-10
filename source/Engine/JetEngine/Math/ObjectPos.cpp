/****************************************************************************************/
/*  OBJECTPOS.C                                                                         */
/*                                                                                      */
/*  Author: David Eisele	                                                        */
/*  Description: Simplify and improve rotation and tranlation of objects                */
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
/*  This file was not part of the original Jet3D, released December 12, 1999.           */
/*                                                                                      */
/****************************************************************************************/

#include <Assert.h>
#include <Math.h>

#include "ObjectPos.h"

#ifndef NDEBUG
	static jeBoolean jeObjectPos_MaximalAssertionMode = JE_TRUE;
	#define jeObjectPos_Assert if (jeObjectPos_MaximalAssertionMode) assert

JETAPI 	void JETCC jeObjectPos_SetMaximalAssertionMode( jeBoolean Enable )
	{
		assert( (Enable == JE_TRUE) || (Enable == JE_FALSE) );
		jeObjectPos_MaximalAssertionMode = Enable;
	}
#else
	#define jeObjectPos_Assert(x)
#endif

#define JE_OBJECTPOS_NORMAL			(1<<0)		// BMatrix is identity
#define JE_OBJECTPOS_XFORMED		(1<<1)		// BMatrix is NOT identity

// -----------------------------------------------------------------------------------------------
// Matrix34 Methods
// -----------------------------------------------------------------------------------------------

jeBoolean JETCC Matrix34_IsValid(const Matrix34 *M)
	// returns JE_TRUE if M is 'valid'  
	// 'valid' means that M is non NULL, and there are no NAN's in the matrix.
{
	if (M == NULL)
		return JE_FALSE;

	if ((M->x[0][0] * M->x[0][0]) < 0.0f) 
		return JE_FALSE;
	if ((M->x[0][1] * M->x[0][1]) < 0.0f) 
		return JE_FALSE;
	if ((M->x[0][2] * M->x[0][2]) < 0.0f) 
		return JE_FALSE;

	if ((M->x[1][0] * M->x[1][0]) < 0.0f) 
		return JE_FALSE;
	if ((M->x[1][1] * M->x[1][1]) < 0.0f) 
		return JE_FALSE;
	if ((M->x[1][2] * M->x[1][2]) < 0.0f) 
		return JE_FALSE;
	
	if ((M->x[2][0] * M->x[2][0]) < 0.0f) 
		return JE_FALSE;
	if ((M->x[2][1] * M->x[2][1]) < 0.0f) 
		return JE_FALSE;
	if ((M->x[2][2] * M->x[2][2]) < 0.0f) 
		return JE_FALSE;

	return JE_TRUE;
}

void JETCC Matrix34_Multiply(const Matrix34 *M1, const Matrix34 *M2, Matrix34 *M12)
{
	#define FSIZE	4
	_asm {
		mov   esi,M1
		mov	  edi,M2
		mov   ebx,M12
		mov   ecx,3

RowMult:
		fld   dword ptr [esi+2*FSIZE]			// 1;M1_i2
		fld   dword ptr [esi+1*FSIZE]			// 1;M1_i2 | M1_i1
		fld   dword ptr [esi+0*FSIZE]			// 1;M1_i2 | M1_i1 | M1_i0 
		fld   st(0)								// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0
		fmul  dword ptr [edi+(0+0*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00
		fld   st(2)								// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00 | M1_i1
		fmul  dword ptr [edi+(0+1*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00 | M1_i1*M2_10
		faddp st(1),st							// 2;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00+M1_i1*M2_10
		fld   st(3)								// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00+M1_i1*M2_10 | M1_i2
		fmul  dword ptr [edi+(0+2*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00+M1_i1*M2_10 | M1_i2*M2_20
		faddp st(1),st							// 2;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_00+M1_i1*M2_10+M1_i2*M2_20
		fstp   dword ptr [ebx+0*FSIZE]			// 2;M1_i2 | M1_i1 | M1_i0
		fld   st(0)								// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0
		fmul  dword ptr [edi+(1+0*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01
		fld   st(2)								// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01 | M1_i1
		fmul  dword ptr [edi+(1+1*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01 | M1_i1*M2_11
		faddp st(1),st							// 2;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01+M1_i1*M2_11
		fld   st(3)								// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01+M1_i1*M2_11 | M1_i2
		fmul  dword ptr [edi+(1+2*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01+M1_i1*M2_11 | M1_i2*M2_21
		faddp st(1),st							// 2;M1_i2 | M1_i1 | M1_i0 | M1_i0*M2_01+M1_i1*M2_11+M1_i2*M2_21
		fstp   dword ptr [ebx+1*FSIZE]			// 2;M1_i2 | M1_i1 | M1_i0
		fmul  dword ptr [edi+(2+0*4)*FSIZE]		// 1;M1_i2 | M1_i1 | M1_i0*M2_02
		fxch  st(1)								// 0;M1_i2 | M1_i0*M2_02 | M1_i1
		fmul  dword ptr [edi+(2+1*4)*FSIZE]		// 1;M1_i2 | M1_i0*M2_02 | M1_i1*M2_12
		faddp st(1),st							// 2;M1_i2 | M1_i0*M2_02+M1_i1*M2_12
		fxch  st(1)								// 0;M1_i0*M2_02+M1_i1*M2_12 | M1_i2
		fmul  dword ptr [edi+(2+2*4)*FSIZE]		// 1;M1_i0*M2_02+M1_i1*M2_12 | M1_i2*M2_22
		faddp st(1),st							// 2;M1_i0*M2_02+M1_i1*M2_12+M1_i2*M2_22
		fstp   dword ptr [ebx+2*FSIZE]			// 2;

		mov   dword ptr [ebx+3*FSIZE],0			// Set Pads&Flags=0 (Matrix34 always ORTHOGNORMAL!)
		add   esi,4*FSIZE
		add   ebx,4*FSIZE
		loop  RowMult							// i=i+1
	}											// ca. 3*3*12+8=116 cycles, dunno exact cycles/command
												// Any improvement possible?
}

void JETCC ASM_RecalcRotation(jeObjectPos *APos)
{
/*	APos->SPhi=jeFloat_Sin(APos->Phi);
	APos->CPhi=jeFloat_Cos(APos->Phi);

	APos->SPsi=jeFloat_Sin(APos->Psi);
	APos->CPsi=jeFloat_Cos(APos->Psi);

	APos->SRho=jeFloat_Sin(APos->Rho);
	APos->CRho=jeFloat_Cos(APos->Rho);

	APos->RMatrix.x[0][0]= CPhi * CRho + SPhi * SPsi * SRho;
	APos->RMatrix.x[0][1]=-CPhi * SRho + SPhi * SPsi * CRho;
	APos->RMatrix.x[0][2]= SPhi * CPsi;

	APos->RMatrix.x[1][0]= CPsi * SRho;
	APos->RMatrix.x[1][1]= CPsi * CRho;
	APos->RMatrix.x[1][2]=-SPsi;

	APos->RMatrix.x[2][0]=-SPhi * CRho + CPhi * SPsi * SRho;
	APos->RMatrix.x[2][1]= SPhi * SRho + CPhi * SPsi * CRho;
	APos->RMatrix.x[2][2]= CPhi * CPsi;*/
	#define FSIZE		4
	#define PHI			esi+0*FSIZE
	#define PSI			esi+1*FSIZE
	#define RHO			esi+2*FSIZE
	#define CPHI		esi+3*FSIZE
	#define SPHI		esi+4*FSIZE
	#define CPSI		esi+5*FSIZE
	#define SPSI		esi+6*FSIZE
	#define CRHO		esi+7*FSIZE
	#define SRHO		esi+8*FSIZE
	#define MATRIX(y,x) esi+(9+(x+y*4))*FSIZE
	_asm {
		mov   esi,APos
		mov   dword ptr [esi+12*FSIZE],0
		mov   dword ptr [esi+16*FSIZE],0
		mov   dword ptr [esi+20*FSIZE],0

		fld   dword ptr [PHI]			// 1  ; Phi
		fsincos							// 250; SPhi | CPhi
		fstp  dword ptr [CPHI]			// 2  ; SPhi
		fstp  dword ptr [SPHI]			// 2  ;
		fld   dword ptr [PSI]			// 1  ; Psi
		fsincos							// 250; SPsi | CPsi
		fstp  dword ptr [CPSI]			// 2  ; SPsi
		fstp  dword ptr [SPSI]			// 2  ;
		fld   dword ptr [RHO]			// 1  ; Rho
		fsincos							// 250; SRho | CRho
		fstp  dword ptr [CRHO]			// 2  ; SRho
		fstp  dword ptr [SRHO]			// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*CRho | SPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*CRho | SPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*CRho | SPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*CRho | SPhi*SPsi | SPhi*SPsi*SRho
		faddp st(2),st					// 2  ; CPhi*CRho+SPhi*SPsi*SRho | SPhi*SPsi
		fxch  st(1)						// 0  ; SPhi*SPsi | CPhi*CRho+SPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(0,0)]	// 2  ; SPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; SPhi*SPsi*CRho
		fld   dword ptr [CPHI]			// 1  ; SPhi*SPsi*CRho | CPhi
		fmul  dword ptr [SRHO]			// 1  ; SPhi*SPsi*CRho | CPhi*SRho
		fsubp st(1),st					// 2  ; -CPhi*SRho+SPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(0,1)]	// 2  ; 
		fld   dword ptr [SPHI]			// 1  ; SPhi 
		fmul  dword ptr [CPSI]			// 1  ; SPhi*CPsi
		fstp  dword ptr [MATRIX(0,2)]	// 2  ; 
		fld   dword ptr [CPSI]			// 1  ; CPsi
		fmul  dword ptr [SRHO]			// 1  ; CPsi*SRho
		fstp  dword ptr [MATRIX(1,0)]	// 2  ; 
		fld   dword ptr [CPSI]			// 1  ; CPsi
		fmul  dword ptr [CRHO]			// 1  ; CPsi*CRho
		fstp  dword ptr [MATRIX(1,1)]	// 2  ; 
		fldz							// 0  ; 0
		fsub  dword ptr [SPSI]			// 1  ; -SPsi
		fstp  dword ptr [MATRIX(1,2)]	// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*SPsi | CPhi*SPsi*SRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi*CRho
		fsubp st(1),st					// 2  ; CPhi*SPsi | -SPhi*CRho+CPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(2,0)]	// 2  ; CPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi*CRho | SPhi
		fmul  dword ptr [SRHO]			// 1  ; CPhi*SPsi*CRho | SPhi*SRho
		faddp st(1),st					// 2  ; SPhi*SRho+CPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(2,1)]	// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CPSI]			// 1  ; CPhi*CPsi
		fstp  dword ptr [MATRIX(2,2)]	// 2  ;
	}											// ca. 4+3*255+2*21+11=824 cycles, dunno exact cycles/command
												// Any improvement possible?
	if (APos->Flags&JE_OBJECTPOS_XFORMED)
		Matrix34_Multiply(&APos->RMatrix,&APos->BMatrix,&APos->Matrix);
	else 
		APos->Matrix=APos->RMatrix;
}

void JETCC ASM_RecalcSpin(jeObjectPos *APos)
{
/*	APos->SPhi=jeFloat_Sin(APos->Phi);
	APos->CPhi=jeFloat_Cos(APos->Phi);

	APos->RMatrix.x[0][0]= APos->CPhi * APos->CRho + APos->SPhi * APos->SPsi * APos->SRho;
	APos->RMatrix.x[0][1]=-APos->CPhi * APos->SRho + APos->SPhi * APos->SPsi * APos->CRho;
	APos->RMatrix.x[0][2]= APos->SPhi * APos->CPsi;

	APos->RMatrix.x[2][0]=-APos->SPhi * APos->CRho + APos->CPhi * APos->SPsi * APos->SRho;
	APos->RMatrix.x[2][1]= APos->SPhi * APos->SRho + APos->CPhi * APos->SPsi * APos->CRho;
	APos->RMatrix.x[2][2]= APos->CPhi * APos->CPsi;*/
	#define FSIZE		4
	#define PHI			esi+0*FSIZE
	#define PSI			esi+1*FSIZE
	#define RHO			esi+2*FSIZE
	#define CPHI		esi+3*FSIZE
	#define SPHI		esi+4*FSIZE
	#define CPSI		esi+5*FSIZE
	#define SPSI		esi+6*FSIZE
	#define CRHO		esi+7*FSIZE
	#define SRHO		esi+8*FSIZE
	#define MATRIX(y,x) esi+(9+(x+y*4))*FSIZE
	_asm {
		mov   esi,APos
		mov   dword ptr [esi+12*FSIZE],0
		mov   dword ptr [esi+16*FSIZE],0
		mov   dword ptr [esi+20*FSIZE],0

		fld   dword ptr [PHI]			// 1  ; Phi
		fsincos							// 250; SPhi | CPhi
		fstp  dword ptr [CPHI]			// 2  ; SPhi
		fstp  dword ptr [SPHI]			// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*CRho | SPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*CRho | SPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*CRho | SPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*CRho | SPhi*SPsi | SPhi*SPsi*SRho
		faddp st(2),st					// 2  ; CPhi*CRho+SPhi*SPsi*SRho | SPhi*SPsi
		fxch  st(1)						// 0  ; SPhi*SPsi | CPhi*CRho+SPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(0,0)]	// 2  ; SPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; SPhi*SPsi*CRho
		fld   dword ptr [CPHI]			// 1  ; SPhi*SPsi*CRho | CPhi
		fmul  dword ptr [SRHO]			// 1  ; SPhi*SPsi*CRho | CPhi*SRho
		fsubp st(1),st					// 2  ; -CPhi*SRho+SPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(0,1)]	// 2  ; 
		fld   dword ptr [SPHI]			// 1  ; SPhi 
		fmul  dword ptr [CPSI]			// 1  ; SPhi*CPsi
		fstp  dword ptr [MATRIX(0,2)]	// 2  ; 
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*SPsi | CPhi*SPsi*SRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi*CRho
		fsubp st(1),st					// 2  ; CPhi*SPsi | -SPhi*CRho+CPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(2,0)]	// 2  ; CPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi*CRho | SPhi
		fmul  dword ptr [SRHO]			// 1  ; CPhi*SPsi*CRho | SPhi*SRho
		faddp st(1),st					// 2  ; SPhi*SRho+CPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(2,1)]	// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CPSI]			// 1  ; CPhi*CPsi
		fstp  dword ptr [MATRIX(2,2)]	// 2  ;
	}											// ca. 4+255+2*21=301 cycles, dunno exact cycles/command
												// Any improvement possible?
	if (APos->Flags&JE_OBJECTPOS_XFORMED)
		Matrix34_Multiply(&APos->RMatrix,&APos->BMatrix,&APos->Matrix);
	else 
		APos->Matrix=APos->RMatrix;
}

void JETCC ASM_RecalcSlope(jeObjectPos *APos)
{
/*	APos->SPsi=jeFloat_Sin(APos->Psi);
	APos->CPsi=jeFloat_Cos(APos->Psi);

	APos->RMatrix.x[0][0]= CPhi * CRho + SPhi * SPsi * SRho;
	APos->RMatrix.x[0][1]=-CPhi * SRho + SPhi * SPsi * CRho;
	APos->RMatrix.x[0][2]= SPhi * CPsi;

	APos->RMatrix.x[1][0]= CPsi * SRho;
	APos->RMatrix.x[1][1]= CPsi * CRho;
	APos->RMatrix.x[1][2]=-SPsi;

	APos->RMatrix.x[2][0]=-SPhi * CRho + CPhi * SPsi * SRho;
	APos->RMatrix.x[2][1]= SPhi * SRho + CPhi * SPsi * CRho;
	APos->RMatrix.x[2][2]= CPhi * CPsi;*/
	#define FSIZE		4
	#define PHI			esi+0*FSIZE
	#define PSI			esi+1*FSIZE
	#define RHO			esi+2*FSIZE
	#define CPHI		esi+3*FSIZE
	#define SPHI		esi+4*FSIZE
	#define CPSI		esi+5*FSIZE
	#define SPSI		esi+6*FSIZE
	#define CRHO		esi+7*FSIZE
	#define SRHO		esi+8*FSIZE
	#define MATRIX(y,x) esi+(9+(x+y*4))*FSIZE
	_asm {
		mov   esi,APos
		mov   dword ptr [esi+12*FSIZE],0
		mov   dword ptr [esi+16*FSIZE],0
		mov   dword ptr [esi+20*FSIZE],0

		fld   dword ptr [PSI]			// 1  ; Psi
		fsincos							// 250; SPsi | CPsi
		fstp  dword ptr [CPSI]			// 2  ; SPsi
		fstp  dword ptr [SPSI]			// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*CRho | SPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*CRho | SPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*CRho | SPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*CRho | SPhi*SPsi | SPhi*SPsi*SRho
		faddp st(2),st					// 2  ; CPhi*CRho+SPhi*SPsi*SRho | SPhi*SPsi
		fxch  st(1)						// 0  ; SPhi*SPsi | CPhi*CRho+SPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(0,0)]	// 2  ; SPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; SPhi*SPsi*CRho
		fld   dword ptr [CPHI]			// 1  ; SPhi*SPsi*CRho | CPhi
		fmul  dword ptr [SRHO]			// 1  ; SPhi*SPsi*CRho | CPhi*SRho
		fsubp st(1),st					// 2  ; -CPhi*SRho+SPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(0,1)]	// 2  ; 
		fld   dword ptr [SPHI]			// 1  ; SPhi 
		fmul  dword ptr [CPSI]			// 1  ; SPhi*CPsi
		fstp  dword ptr [MATRIX(0,2)]	// 2  ; 
		fld   dword ptr [CPSI]			// 1  ; CPsi
		fmul  dword ptr [SRHO]			// 1  ; CPsi*SRho
		fstp  dword ptr [MATRIX(1,0)]	// 2  ; 
		fld   dword ptr [CPSI]			// 1  ; CPsi
		fmul  dword ptr [CRHO]			// 1  ; CPsi*CRho
		fstp  dword ptr [MATRIX(1,1)]	// 2  ; 
		fldz							// 0  ; 0
		fsub  dword ptr [SPSI]			// 1  ; -SPsi
		fstp  dword ptr [MATRIX(1,2)]	// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*SPsi | CPhi*SPsi*SRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi*CRho
		fsubp st(1),st					// 2  ; CPhi*SPsi | -SPhi*CRho+CPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(2,0)]	// 2  ; CPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi*CRho | SPhi
		fmul  dword ptr [SRHO]			// 1  ; CPhi*SPsi*CRho | SPhi*SRho
		faddp st(1),st					// 2  ; SPhi*SRho+CPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(2,1)]	// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CPSI]			// 1  ; CPhi*CPsi
		fstp  dword ptr [MATRIX(2,2)]	// 2  ;
	}											// ca. 4+255+2*21+11=312 cycles, dunno exact cycles/command
												// Any improvement possible?
	if (APos->Flags&JE_OBJECTPOS_XFORMED)
		Matrix34_Multiply(&APos->RMatrix,&APos->BMatrix,&APos->Matrix);
	else 
		APos->Matrix=APos->RMatrix;
}

void JETCC ASM_RecalcTilt(jeObjectPos *APos)
{
/*	APos->SRho=jeFloat_Sin(APos->Rho);
	APos->CRho=jeFloat_Cos(APos->Rho);

	APos->RMatrix.x[0][0]= APos->CPhi * APos->CRho + APos->SPhi * APos->SPsi * APos->SRho;
	APos->RMatrix.x[0][1]=-APos->CPhi * APos->SRho + APos->SPhi * APos->SPsi * APos->CRho;

	APos->RMatrix.x[1][0]= APos->CPsi * APos->SRho;
	APos->RMatrix.x[1][1]= APos->CPsi * APos->CRho;

	APos->RMatrix.x[2][0]=-APos->SPhi * APos->CRho + APos->CPhi * APos->SPsi * APos->SRho;
	APos->RMatrix.x[2][1]= APos->SPhi * APos->SRho + APos->CPhi * APos->SPsi * APos->CRho;*/
	#define FSIZE		4
	#define PHI			esi+0*FSIZE
	#define PSI			esi+1*FSIZE
	#define RHO			esi+2*FSIZE
	#define CPHI		esi+3*FSIZE
	#define SPHI		esi+4*FSIZE
	#define CPSI		esi+5*FSIZE
	#define SPSI		esi+6*FSIZE
	#define CRHO		esi+7*FSIZE
	#define SRHO		esi+8*FSIZE
	#define MATRIX(y,x) esi+(9+(x+y*4))*FSIZE
	_asm {
		mov   esi,APos
		mov   dword ptr [esi+12*FSIZE],0
		mov   dword ptr [esi+16*FSIZE],0
		mov   dword ptr [esi+20*FSIZE],0

		fld   dword ptr [RHO]			// 1  ; Rho
		fsincos							// 250; SRho | CRho
		fstp  dword ptr [CRHO]			// 2  ; SRho
		fstp  dword ptr [SRHO]			// 2  ;
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*CRho | SPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*CRho | SPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*CRho | SPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*CRho | SPhi*SPsi | SPhi*SPsi*SRho
		faddp st(2),st					// 2  ; CPhi*CRho+SPhi*SPsi*SRho | SPhi*SPsi
		fxch  st(1)						// 0  ; SPhi*SPsi | CPhi*CRho+SPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(0,0)]	// 2  ; SPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; SPhi*SPsi*CRho
		fld   dword ptr [CPHI]			// 1  ; SPhi*SPsi*CRho | CPhi
		fmul  dword ptr [SRHO]			// 1  ; SPhi*SPsi*CRho | CPhi*SRho
		fsubp st(1),st					// 2  ; -CPhi*SRho+SPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(0,1)]	// 2  ; 
		fld   dword ptr [CPSI]			// 1  ; CPsi
		fmul  dword ptr [SRHO]			// 1  ; CPsi*SRho
		fstp  dword ptr [MATRIX(1,0)]	// 2  ; 
		fld   dword ptr [CPSI]			// 1  ; CPsi
		fmul  dword ptr [CRHO]			// 1  ; CPsi*CRho
		fstp  dword ptr [MATRIX(1,1)]	// 2  ; 
		fld   dword ptr [CPHI]			// 1  ; CPhi
		fmul  dword ptr [SPSI]			// 1  ; CPhi*SPsi
		fld   dword ptr [SRHO]			// 1  ; CPhi*SPsi | SRho
		fmul  st,st(1)					// 1  ; CPhi*SPsi | CPhi*SPsi*SRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi | CPhi*SPsi*SRho | SPhi*CRho
		fsubp st(1),st					// 2  ; CPhi*SPsi | -SPhi*CRho+CPhi*SPsi*SRho
		fstp  dword ptr [MATRIX(2,0)]	// 2  ; CPhi*SPsi
		fmul  dword ptr [CRHO]			// 1  ; CPhi*SPsi*CRho
		fld   dword ptr [SPHI]			// 1  ; CPhi*SPsi*CRho | SPhi
		fmul  dword ptr [SRHO]			// 1  ; CPhi*SPsi*CRho | SPhi*SRho
		faddp st(1),st					// 2  ; SPhi*SRho+CPhi*SPsi*CRho
		fstp  dword ptr [MATRIX(2,1)]	// 2  ;
	}											// ca. 4+255+2*21=301 cycles, dunno exact cycles/command
												// Any improvement possible?
	if (APos->Flags&JE_OBJECTPOS_XFORMED)
		Matrix34_Multiply(&APos->RMatrix,&APos->BMatrix,&APos->Matrix);
	else 
		APos->Matrix=APos->RMatrix;
}

void JETCC Matrix34_SetIdentity(Matrix34 *M)
{
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 4; j++)
		{
			if (i == j) 
				M->x[i][j] = 1.f;
			else M->x[i][j] = 0.f;
		}
}

void JETCC Matrix34_CutZeroOne(Matrix34 *M)
{
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 4; j++)
		{
			if (JE_FLOATS_EQUAL(M->x[i][j],1.0f))
				M->x[i][j] = 1.0f;
			else if (JE_FLOAT_ISZERO(M->x[i][j]))
				M->x[i][j] = 0.0f;
		}
}

// -----------------------------------------------------------------------------------------------
// Init-Methods
// -----------------------------------------------------------------------------------------------

JETAPI void JETCC jeObjectPos_SetIdentity(jeObjectPos *APos)
{
	assert(APos!=NULL);
	APos->Phi=APos->Psi=APos->Rho=APos->SPhi=APos->SPsi=APos->SRho=0.0f;
	APos->CPhi=APos->CPsi=APos->CRho=1.0f;
	APos->Translation.X=APos->Translation.Y=APos->Translation.Z=0.0f;
	Matrix34_SetIdentity(&APos->RMatrix);
	APos->Matrix=APos->RMatrix;
	APos->Flags=JE_OBJECTPOS_NORMAL;
}

JETAPI void JETCC jeObjectPos_SetBaseXForm(jeObjectPos *APos, const jeXForm3d *BaseXF)
{
	jeObjectPos_SetIdentity(APos);
	jeObjectPos_SetNewBaseXForm(APos,BaseXF);
}

JETAPI void JETCC jeObjectPos_SetBaseXFormByAxis(jeObjectPos *APos, const jeVec3d *X, const jeVec3d *Y, const jeVec3d *Z)
{
	jeObjectPos_SetIdentity(APos);
	jeObjectPos_SetNewBaseXFormByAxis(APos,X,Y,Z);
}

JETAPI void JETCC jeObjectPos_SetTranslation(jeObjectPos *APos, jeFloat X, jeFloat Y, jeFloat Z)
{
	assert(APos!=NULL);
	assert(X*X>=0.0f);
	assert(Y*Y>=0.0f);
	assert(Z*Z>=0.0f);
	APos->Phi=APos->Psi=APos->Rho=APos->SPhi=APos->SPsi=APos->SRho=0;
	APos->CPhi=APos->CPsi=APos->CRho=1.0f;
	APos->Translation.X=X;APos->Translation.Y=Y;APos->Translation.Z=Z;
	Matrix34_SetIdentity(&APos->RMatrix);
	APos->Matrix=APos->RMatrix;
	APos->Flags=JE_OBJECTPOS_NORMAL;
}

JETAPI void JETCC jeObjectPos_SetTranslationByVec(jeObjectPos *APos, jeVec3d *T)
{
	assert(T!=NULL);
	jeObjectPos_SetTranslation(APos,T->X,T->Y,T->Z);
}

JETAPI void JETCC jeObjectPos_SetRotation(jeObjectPos *APos, jeFloat Phi, jeFloat Psi, jeFloat Rho)
{
	assert(APos!=NULL);
	assert(Phi*Phi>=0.0f);
	assert(Psi*Psi>=0.0f);
	assert(Rho*Rho>=0.0f);

	APos->Phi=Phi;APos->Psi=Psi;APos->Rho=Rho;
	APos->Flags=JE_OBJECTPOS_NORMAL;
	ASM_RecalcRotation(APos);
	APos->Translation.X=APos->Translation.Y=APos->Translation.Z=0.0f;
}


// -----------------------------------------------------------------------------------------------
// Set-Methods
// -----------------------------------------------------------------------------------------------

JETAPI void JETCC jeObjectPos_SetNewRotation(jeObjectPos *APos, jeFloat Phi, jeFloat Psi, jeFloat Rho)
{
	assert(APos!=NULL);
	assert(Phi*Phi>=0.0f);
	assert(Psi*Psi>=0.0f);
	assert(Rho*Rho>=0.0f);

	APos->Phi=Phi;APos->Psi=Psi;APos->Rho=Rho;
	ASM_RecalcRotation(APos);
}

JETAPI void JETCC jeObjectPos_SetNewBaseXForm(jeObjectPos *APos, const jeXForm3d *BaseXF)
{
	assert(APos!=NULL);assert(BaseXF!=NULL);
	jeObjectPos_Assert(jeXForm3d_IsOrthonormal(BaseXF));
	jeObjectPos_Assert(Matrix34_IsValid(&APos->RMatrix));

	APos->BMatrix=((MatrixXForm*)BaseXF)->Matrix;
	Matrix34_CutZeroOne(&APos->BMatrix);
	if ((APos->BMatrix.x[0][0]==1.0f) ||
		(APos->BMatrix.x[1][1]==1.0f) ||
		(APos->BMatrix.x[2][2]==1.0f) ||
		(APos->BMatrix.x[0][1]==0.0f) ||
		(APos->BMatrix.x[0][2]==0.0f) ||
		(APos->BMatrix.x[1][0]==0.0f) ||
		(APos->BMatrix.x[1][2]==0.0f) ||
		(APos->BMatrix.x[2][0]==0.0f) ||
		(APos->BMatrix.x[2][1]==0.0f)) {
		APos->Flags=JE_OBJECTPOS_XFORMED;
		Matrix34_Multiply(&APos->RMatrix,&APos->BMatrix,&APos->Matrix);
	} else {
		APos->Flags=JE_OBJECTPOS_NORMAL;
		APos->Matrix=APos->RMatrix;
	}
}

JETAPI void JETCC jeObjectPos_SetNewBaseXFormByAxis(jeObjectPos *APos, const jeVec3d *X, const jeVec3d *Y, const jeVec3d *Z)
{
	assert(APos!=NULL);assert(X!=NULL);assert(Y!=NULL);assert(Z!=NULL);
	// ASSERT Vectors are Orthonormal
	jeObjectPos_Assert(jeVec3d_IsNormalized(X) && jeVec3d_IsNormalized(Y) && jeVec3d_IsNormalized(Z));
	jeObjectPos_Assert(JE_FLOAT_ISZERO(jeVec3d_DotProduct(X,Y)) &&
					   JE_FLOAT_ISZERO(jeVec3d_DotProduct(X,Z)) &&
					   JE_FLOAT_ISZERO(jeVec3d_DotProduct(Y,Z)));
	jeObjectPos_Assert(Matrix34_IsValid(&APos->RMatrix));
	
	APos->BMatrix.x[0][0]=-X->X;
	APos->BMatrix.x[1][0]=-X->Y;
	APos->BMatrix.x[2][0]=-X->Z;
	APos->BMatrix.x[0][1]=Y->X;
	APos->BMatrix.x[1][1]=Y->Y;
	APos->BMatrix.x[2][1]=Y->Z;
	APos->BMatrix.x[0][2]=-Z->X;
	APos->BMatrix.x[1][2]=-Z->Y;
	APos->BMatrix.x[2][2]=-Z->Z;
	
	if (!JE_FLOATS_EQUAL(X->X,1.0f) ||
		!JE_FLOATS_EQUAL(Y->Y,1.0f) ||
		!JE_FLOATS_EQUAL(Z->Z,1.0f) ||
		!JE_FLOAT_ISZERO(X->Y) ||
		!JE_FLOAT_ISZERO(X->Z) ||
		!JE_FLOAT_ISZERO(Y->X) ||
		!JE_FLOAT_ISZERO(Y->Z) ||
		!JE_FLOAT_ISZERO(Z->X) ||
		!JE_FLOAT_ISZERO(Z->Y)) {
		APos->Flags=JE_OBJECTPOS_XFORMED;
		Matrix34_Multiply(&APos->RMatrix,&APos->BMatrix,&APos->Matrix);
	} else {
		APos->Flags=JE_OBJECTPOS_NORMAL;
		APos->Matrix=APos->RMatrix;
	}
}

// -----------------------------------------------------------------------------------------------
// Transform-Methods
// -----------------------------------------------------------------------------------------------

JETAPI void JETCC jeObjectPos_Spin(jeObjectPos *APos, jeFloat DPhi)
{
	assert(APos!=NULL);
	assert(DPhi*DPhi>=0.0f);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));

	APos->Phi+=DPhi;
	ASM_RecalcSpin(APos);

	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_Slope(jeObjectPos *APos, jeFloat DPsi)
{
	assert(APos!=NULL);
	assert(DPsi*DPsi>=0.0f);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));

	APos->Psi+=DPsi;
	ASM_RecalcSlope(APos);

	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_Tilt(jeObjectPos *APos, jeFloat DRho)
{
	assert(APos!=NULL);
	assert(DRho*DRho>=0.0f);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));

	APos->Rho+=DRho;
	ASM_RecalcTilt(APos);

	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_Rotate(jeObjectPos *APos, jeFloat DPhi, jeFloat DPsi, jeFloat DRho)
{

	assert(APos!=NULL);
	assert(DPhi*DPhi>=0.0f);
	assert(DPsi*DPsi>=0.0f);
	assert(DRho*DRho>=0.0f);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));

	APos->Phi+=DPhi;
	APos->Psi+=DPsi;
	APos->Rho+=DRho;
	ASM_RecalcRotation(APos);

	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_Translate(jeObjectPos *APos, jeFloat DX, jeFloat DY, jeFloat DZ)
{
	assert(APos!=NULL);
	assert(DX*DX>=0.0f);
	assert(DY*DY>=0.0f);
	assert(DZ*DZ>=0.0f);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	APos->Translation.X+=DX;
	APos->Translation.Y+=DY;
	APos->Translation.Z+=DZ;
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

// -----------------------------------------------------------------------------------------------
// "Move"-Methods
// -----------------------------------------------------------------------------------------------

JETAPI void JETCC jeObjectPos_Move(jeObjectPos *APos, jeVec3d *Dir, jeFloat Dist)
{
	assert(APos!=NULL);
	assert(Dist*Dist>=0.0f);
	assert(jeVec3d_IsValid(Dir)!=JE_FALSE);
	
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	APos->Translation.X+=Dist*Dir->X;
	APos->Translation.Y+=Dist*Dir->Y;
	APos->Translation.Z+=Dist*Dir->Z;
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_MoveIn(jeObjectPos *APos, jeFloat Dist)
{
	assert(APos!=NULL);
	assert(Dist*Dist>=0.0f);
	
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	APos->Translation.X-=Dist*APos->RMatrix.x[0][2]; // -AZ
	APos->Translation.Y-=Dist*APos->RMatrix.x[1][2]; // -BZ
	APos->Translation.Z-=Dist*APos->RMatrix.x[2][2]; // -CZ
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_MoveUp(jeObjectPos *APos, jeFloat Dist)
{
	assert(APos!=NULL);
	assert(Dist*Dist>=0.0f);
	
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	APos->Translation.X+=Dist*APos->RMatrix.x[0][1]; // AY 
	APos->Translation.Y+=Dist*APos->RMatrix.x[1][1]; // BY
	APos->Translation.Z+=Dist*APos->RMatrix.x[2][1]; // BZ
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_MoveLeft(jeObjectPos *APos, jeFloat Dist)
{
	assert(APos!=NULL);
	assert(Dist*Dist>=0.0f);
	
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	APos->Translation.X-=Dist*APos->RMatrix.x[0][0]; // -AX
	APos->Translation.Y-=Dist*APos->RMatrix.x[1][0]; // -BX
	APos->Translation.Z-=Dist*APos->RMatrix.x[2][0]; // -CX
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
}

JETAPI void JETCC jeObjectPos_MoveByDifference(const jeObjectPos *OPos, const jeObjectPos *NPos, jeObjectPos *APos)
{
	jeFloat		DX,DY,DZ;

	assert(OPos!=NULL);assert(NPos!=NULL);assert(APos!=NULL);
	jeObjectPos_Assert(jeObjectPos_IsValid(OPos));
	jeObjectPos_Assert(jeObjectPos_IsValid(NPos));
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));

	DX=NPos->Translation.X-OPos->Translation.X;
	if (JE_FLOAT_ISZERO(DX)) DX=0.0f;

	DY=NPos->Translation.Y-OPos->Translation.Y;
	if (JE_FLOAT_ISZERO(DY)) DY=0.0f;

	DZ=NPos->Translation.Z-OPos->Translation.Z;
	if (JE_FLOAT_ISZERO(DZ)) DZ=0.0f;

	jeObjectPos_Translate(APos,DX,DY,DZ);
}

JETAPI void JETCC jeObjectPos_TransformByDifference(const jeObjectPos *OPos, const jeObjectPos *NPos, jeObjectPos *APos)
{
	jeFloat		DPhi,DPsi,DRho,DX,DY,DZ;
	jeBoolean	PhiChange,PsiChange,RhoChange;

	assert(OPos!=NULL);assert(NPos!=NULL);assert(APos!=NULL);
	jeObjectPos_Assert(jeObjectPos_IsValid(OPos));
	jeObjectPos_Assert(jeObjectPos_IsValid(NPos));
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));

	DPhi=NPos->Phi-OPos->Phi;
	PhiChange=!JE_FLOAT_ISZERO(DPhi);
	if (!PhiChange) DPhi=0.0f;

	DPsi=NPos->Psi-OPos->Psi;
	PsiChange=!JE_FLOAT_ISZERO(DPsi);
	if (!PsiChange) DPsi=0.0f;

	DRho=NPos->Rho-OPos->Rho;
	RhoChange=!JE_FLOAT_ISZERO(DRho);
	if (!RhoChange) DRho=0.0f;

	if (PsiChange || (PhiChange && RhoChange)) {
		jeObjectPos_Rotate(APos,DPhi,DPsi,DRho);
	} else if (PhiChange) {
		jeObjectPos_Spin(APos,DPhi);
	} else if (RhoChange) {
		jeObjectPos_Tilt(APos,DRho);
	}

	DX=NPos->Translation.X-OPos->Translation.X;
	if (JE_FLOAT_ISZERO(DX)) DX=0.0f;

	DY=NPos->Translation.Y-OPos->Translation.Y;
	if (JE_FLOAT_ISZERO(DY)) DY=0.0f;

	DZ=NPos->Translation.Z-OPos->Translation.Z;
	if (JE_FLOAT_ISZERO(DZ)) DZ=0.0f;

	jeObjectPos_Translate(APos,DX,DY,DZ);
}

// -----------------------------------------------------------------------------------------------
// Query-Methods
// -----------------------------------------------------------------------------------------------

JETAPI void JETCC jeObjectPos_GetIn(const jeObjectPos *APos, jeVec3d *V)
{
	assert(APos!=NULL);assert(V!=NULL);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	V->X=-APos->RMatrix.x[0][2]; // -AZ
	V->Y=-APos->RMatrix.x[1][2]; // -BZ
	V->Z=-APos->RMatrix.x[2][2]; // -CZ
}

JETAPI void JETCC jeObjectPos_GetUp(const jeObjectPos *APos, jeVec3d *V)
{
	assert(APos!=NULL);assert(V!=NULL);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	V->X=APos->RMatrix.x[0][1]; // AY
	V->Y=APos->RMatrix.x[1][1]; // BY
	V->Z=APos->RMatrix.x[2][1]; // CY
}

JETAPI void JETCC jeObjectPos_GetLeft(const jeObjectPos *APos, jeVec3d *V)
{
	assert(APos!=NULL);assert(V!=NULL);
	jeObjectPos_Assert(jeObjectPos_IsValid(APos));
	V->X=-APos->RMatrix.x[0][0]; // -AX
	V->Y=-APos->RMatrix.x[1][0]; // -BX
	V->Z=-APos->RMatrix.x[2][0]; // -CX
}

JETAPI jeBoolean JETCC jeObjectPos_IsValid(const jeObjectPos *APos)
{
	if (APos == NULL)
		return JE_FALSE;
	if (!Matrix34_IsValid(&APos->RMatrix))
		return JE_FALSE;
	if ((APos->Flags&JE_OBJECTPOS_XFORMED) && !Matrix34_IsValid(&APos->BMatrix))
		return JE_FALSE;
	if (!jeXForm3d_IsValid(&APos->XForm))
		return JE_FALSE;
	if (((APos->Phi*APos->Phi)<0.0f) || ((APos->CPhi*APos->CPhi)<0.0f) || ((APos->SPhi*APos->SPhi)<0.0f))
		return JE_FALSE;
	if (((APos->Psi*APos->Psi)<0.0f) || ((APos->CPsi*APos->CPsi)<0.0f) || ((APos->SPsi*APos->SPsi)<0.0f))
		return JE_FALSE;
	if (((APos->Rho*APos->Rho)<0.0f) || ((APos->CRho*APos->CRho)<0.0f) || ((APos->SRho*APos->SRho)<0.0f))
		return JE_FALSE;
	return JE_TRUE;
}
