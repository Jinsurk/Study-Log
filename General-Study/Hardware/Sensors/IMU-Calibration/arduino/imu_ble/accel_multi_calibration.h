#pragma once
// Input/output in g; norm calibration, absolute rotation not identified.
constexpr float ACCEL_MULTI_OFFSET[3] = {-0.0100256141f, -0.0146572613f, 0.0256654756f};
constexpr float ACCEL_MULTI_C[3][3] = {
    {0.9965129979f, -0.0003058205f, -0.0087262995f},
    {0.0000000000f, 0.9983658358f, 0.0014641727f},
    {0.0000000000f, 0.0000000000f, 0.9913144365f},
};
