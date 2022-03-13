#include "UnitsVisionTest.h"
using namespace visEval;


int main(void)
{
	UnitsVisionTest getTestSelection;

	UnitsVision visionCalculator;
	visionCalculator.evaluateVision(getTestSelection());

	while (1);
}