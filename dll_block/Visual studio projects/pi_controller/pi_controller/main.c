#include "DllHeader.h"

//Global variables
double kp, ki, ts;
//Map input and output parameters to meaningful names
#define e aState->inputs[0]
#define i aState->states[0]
#define y aState->outputs[0]


DLLEXPORT void plecsSetSizes(struct SimulationSizes* aSizes)
{
   aSizes->numInputs = 1;
   aSizes->numOutputs = 1;
   aSizes->numStates = 1;
   aSizes->numParameters = 3; //number of user parameters passed in
}


//This function is automatically called at the beginning of the simulation
DLLEXPORT void plecsStart(struct SimulationState* aState)
{
	//Example error message box
	if (aState->parameters[0] < 0 )
		aState->errorMessage = "kp cannot be less than 0";
	kp = aState->parameters[0];
	ki = aState->parameters[1];
	ts = aState->parameters[2];
}


//This function is automatically called every sample time
//output is written to DLL output port after the output delay
DLLEXPORT void plecsOutput(struct SimulationState* aState)
{	
	i = i + ts * e;
	y = kp * e + ki * i;
}
 