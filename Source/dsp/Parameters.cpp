/// @file Parameters.cpp
/// @brief Implementation of the Parameters structure

#include <Parameters.h>

Parameters::Parameters(const unsigned int pWinLength, const float pXMax, const float pSClip)
    : winLength(pWinLength)
    , xMax(pXMax)
    , sClip(pSClip)
{
}
