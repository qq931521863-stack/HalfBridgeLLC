#include "DllHeader.h"

//Global variables

//Map input and output parameters to meaningful names

#define Duty		aState->outputs[0]
#define SR_Duty		aState->outputs[1]
#define Iout		aState->inputs[0]
#define Vout		aState->inputs[1]
#define Vout		aState->inputs[2]	
#define State		aState->inputs[3]	



DLLEXPORT void plecsSetSizes(struct SimulationSizes* aSizes)
{
   aSizes->numInputs = 4;
   aSizes->numOutputs = 2;
   aSizes->numStates = 0;
   aSizes->numParameters = 0; //number of user parameters passed in
}


//This function is automatically called at the beginning of the simulation
DLLEXPORT void plecsStart(struct SimulationState* aState)
{

}


//This function is automatically called every sample time
//output is written to DLL output port after the output delay
DLLEXPORT void plecsOutput(struct SimulationState* aState)
{	
	Duty = 0.20;
	SR_Duty = 0.40;
}
 