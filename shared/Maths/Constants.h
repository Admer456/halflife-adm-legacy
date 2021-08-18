#pragma once

// A lot of places in the code use M_PI at the moment, but 
// the plan is to have them all use this eventually
namespace Math
{
	constexpr float Pi = 3.14159265358979323846f;
	constexpr float PiSquared = Pi * Pi;
	constexpr float PiRoot = 1.77245385090551602729f;
}
