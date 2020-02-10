/*
Copyright (c) 2020 Richard King

The stressRefine analysis executable "SRwithMkl" is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

"SRwithMkl" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

The terms of the GNU General Public License are explained in the file COPYING.txt,
also available at <https://www.gnu.org/licenses/>

Note that in it's current form "SRwithMkl" make use of the pardiso sparse solver
in the Intel MKL library, with which it must be linked.
Copyright (c) 2018 Intel Corporation.
You may use and redistribute the Intel MKL library, without modification, provided the conditions of
the Intel Simplified Software license are met:
https://software.intel.com/en-us/license/intel-simplified-software-license

It is perfectly permissable to replace the use of the pardiso software from the MKL library
with an equivalent open-source solver
*/

//////////////////////////////////////////////////////////////////////
//
// SRanalysis.h: interface for the SRmodel class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#define MAXADAPTLOOPS 3
#define SKIPCONTINATION true

#include "SRcoord.h"
#include "SRmaterial.h"
#include "SRbasis.h"
#include "SRelement.h"
#include "SRutil.h"
#include "SRmath.h"
#include "SRstring.h"
#include "SRfile.h"
#include "SRsolver.h"
#include "SRerrorCheck.h"
#include "SRinput.h"
#include "SRpostProcess.h"
#include "SRmodel.h"
#include "SRmklUtil.h"
#include "SRpoundDefines.h"

class SRPassData
{
public:
	SRPassData(){ maxp = 0; neq = 0; err = 0.0; maxsvm = 0.0; };
	int maxp;
	int neq;
	double err;
	double maxsvm;
};

class SRanalysis
{
	friend class SRinput;

public:

	SRanalysis();

	static void TimeStamp();

	void Initialize();
	void CleanUp(bool partial = false);
	void AllocateSmoothFunctionEquations(int n);
	void AllocateDofVectors();
	void NumberEquationsSmooth();
	bool AdaptUniform(int pIteration, bool checkErrorOnly);
	double GetEnforcedDisplacementCoeff(int gfun, int dof);
	double GetDisplacementCoeff(int gfun, int dof);
	double GetDisplacementCoeff(int eq) { return solution.Get(eq); };
	void NumberEquations();
	void allocateSolutionVector();
	void PStats();
	void CreateModel();
	void CalculateElementStiffnesses(bool anyLcsEnfd);
	bool CalculateElementStiffnessesOmp();
	void NumberGlobalFunctions();
	bool Adapt(int pIteration, bool checkErrorOnly = false);
	void Run();
	bool PreProcessPenaltyConstraints();
	void ProcessConstraints();
	void EnforcedDisplacementAssemble(bool doingPrevSolution = false);
	void ProcessForces();
	void ProcessVolumeForces(SRvec3& ResF);
	void ProcessElemVolumeForce(SRvolumeForce* vf, SRelement*elem, SRvec3& ResF, SRdoubleVector& basvec, double* globalForce);
	void NodalMaxDisp(int& nodeUidAtMaxDisp, SRvec3& disp);
	double* ReadElementStiffness(SRelement* elem);
	void AddSoftSprings();
	double CalculateMaxErrorForOutput();

	double* GetElementStiffnessVector();

	double* GetSolutionVector() { return solution.GetVector(); };
	void AddToSolutionVector(int i, double d){ solution.PlusAssign(i, d); };
	void CopyToSolutionVector(int i, double d){ solution.Put(i, d); };
	void CopyToSolutionVector(SRdoubleVector& v){ solution.Copy(v); };

	void AllocateSkipFun(int n){ skipFun.Allocate(n); };

	double GetStressMax(){ return stressMax; };
	double GetStressMaxComp(int i){ return stressMaxComp[i]; };
	int GetNumFunctions(){ return numFunctions; }
	int GetNumEquations(){ return numEquations; }
	int GetNumSmoothEquations(){ return numSmoothEquations; }
	bool IsAnyEnforcedDisplacement(){ return anyEnforcedDisplacement; };
	void SetMaxPorderLowStress(int p){ maxPorderLowStress = p; };
	int GetMaxPorderLowStress(){ return maxPorderLowStress; };
	int GetnumEquations(){ return numEquations; };
	int GetnumSmoothEquations(){ return numSmoothEquations; };
	int GetMaxElemUid(){ return maxElemUid; };
	bool GetFunUncon(int fun){ return (funUncon.Get(fun) == 1); };
	SRvec3* GetMaxStressPos(){ return &maxStressPos; };
	SRnode* GetNodeFromUid(int i);

	void SetStressMax(SRelement* elem, SRvec3& pos, double svm);
	void SetStressMaxComp(double *stressComp);
	double GetStrainMax(){ return strainMax; };
	void SetStrainMax(double e){ strainMax = e; };
	
	void zeroStressMax()
	{
		stressMax = 0.0;
		for (int i = 0; i < 6; i++)
			stressMaxComp[i] = 0.0;
	};
	void PrintStresses(double *stress);
	void SetDetectSacr(bool sacr){ detectSacrificialElements = sacr; };
	void setAdaptLoopMax(int nits);
	void testMapping();
	void checkElementMapping();
	bool checkFlattenedElementHighStress();
	void CheckElementsFitInMemory();
	void SetEdgesToPorder(int p);
	bool GetAnyWedges(){ return anywedges; };
	bool GetAnyBricks(){ return anybricks; };
	bool GetAnyTets(){ return anytets; };
	void GetFileNameFromExtension(char* ext, SRstring& name);
	bool SingStressCheck();
	void setMaxPorder(int maxp){ maxPorderFinalAdapt = maxp; };
	int GetMaxPorder(){ return maxPorder; };
	int getMaxPJump()
	{
		return maxPJump;
	};
	void setMaxPJump(int j){ maxPJump = j; };
	double GetErrorTolerance(){ return ErrorTolerance; };
	void setErrorTolerance(double tol){ ErrorTolerance = tol; };
	void setErrorMax(double error, int eluid, double errorSmoothRaw, double errorFaceJumps);
	bool getAllMatsEquivElast(){ return allMatsEquivElast; };
	void setAllMatsEquivElast(bool tf){ allMatsEquivElast = tf; };
	double getstressUnitConversion(){ return stressUnitConversion; };
	void setstressUnitConversion(double s){ stressUnitConversion = s; };
	double getlengthUnitConversion(){ return lengthUnitConversion; };
	void setlengthUnitConversion(double l){ lengthUnitConversion = l; };
	bool getoutputf06(){ return outputf06; };
	bool getallMatsHaveAllowable(){ return allMatsHaveAllowable; };
	bool getanyMatYielded(){ return anyMatYielded; };
	void setanyMatYielded(bool tf){ anyMatYielded = tf; };


	//1 instance of each utility class:
	SRsolver solver;
	SRpostProcess post;
	SRinput input;
	SRerrorCheck errorChecker;

	SRstring outdir;
	SRfile inputFile;
	SRfile outputFile;
	SRstring fileNameTail;
	SRfile cgxFrdFile;
	SRfile settingsFile;
	SRfile f06File;
	SRfile f06outFile;
	SRstring wkdir;

private:
	bool anyEnforcedDisplacement;
	int numFunctions;
	int maxNumElementFunctions;
	int numEquations;
	int numSmoothEquations;
	int numprocessors;
	bool detectSacrificialElements;
	int maxStressElid;
	bool needSoftSprings;
	double MaxElementMem;
	int elUidAtMaxError;
	double strainMax;
	double stressMaxComp[6];
	int maxElemUid;
	int adaptIt;
	SRvec3 maxStressPos;
	int nodeidAtMaxStressPos;
	double stressUnitConversion;
	SRstring stressUnitstr;
	double lengthUnitConversion;
	SRstring lengthUnitstr;
	bool useUnits;
	bool allMatsHaveAllowable;
	double errorMax;
	double errorSmoothRawAtMax;
	double errorFaceJumpAtMax;

	SRdoubleVector solution;
	SRintVector skipFun;
	SRintVector funUncon;
	SRvector <SRPassData> passData;

	char stringBuffer[MAXLINELENGTH];
	SRfile elementFileEven;
	SRfile elementFileOdd;
	bool anybricks;
	bool anytets;
	bool anywedges;
	bool allConstraintsAsPenalty;
	SRvec3 umaxInModel;
	int nodeuidAtumax;
	bool outputf06;
	bool doEnergySmooth;
	bool pOrderUniform;
	int maxPinModel;

	double maxAllowableAnyActiveMat;
	int numFlattenedElHighStress;
	bool allMatsEquivElast;
	bool anyMatYielded;
	double prevStressMax;
	double stressMax;
	double stressMaxP2;
	double maxsp1;
	double minsp2;
	bool checkForHotSpotatMax;
	bool anyHotSpotElFound;
	int adaptLoopMax;
	int maxPJump;
	int maxPorder;
	int maxPorderFinalAdapt;
	int maxPorderLowStress;
	double ErrorTolerance;


};

