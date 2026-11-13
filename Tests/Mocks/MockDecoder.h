#pragma once

#include <IDecoder.h>

#include <gmock/gmock.h>

#include <vector>

/// GMock IDecoder, so engine tests control what every block decodes to.
class MockDecoder : public IDecoder
{
public:
    MOCK_METHOD(std::size_t, latentDim, (), (const, override));
    MOCK_METHOD(std::size_t, outputDim, (), (const, override));
    MOCK_METHOD(void, decodeInto, (const std::vector<float> &z, std::vector<float> &out), (const, override));
};
