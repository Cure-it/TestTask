#include "UnitsVisionTest.h"
#include <chrono>


using namespace visEval;


int main(void)
{
	UnitsVisionTest getTestSelection;
	UnitsVision visionCalculator;

	auto start = std::chrono::steady_clock::now();
	visionCalculator.evaluateVision(getTestSelection.getRandomGeneration(10000));
	for (int i = 0; i < 1000; i++)
		visionCalculator.evaluateVision();
	//visionCalculator.evaluateVision(getTestSelection());
	auto stop = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration = stop - start;


	while (1);
}