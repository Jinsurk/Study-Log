// Six-face first-pass estimate; input and output in g.
// corrected = (raw - offset) * gain; validate independently.
#pragma once
constexpr float AX_OFFSET_G = -0.011656056f;
constexpr float AX_GAIN = 0.997875391f;
constexpr float AY_OFFSET_G = -0.016844753f;
constexpr float AY_GAIN = 0.999171941f;
constexpr float AZ_OFFSET_G = 0.026057619f;
constexpr float AZ_GAIN = 0.992444909f;
