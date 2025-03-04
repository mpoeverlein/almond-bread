#include <algorithm>

// Function written by Deepseek
/**
 * @brief
 * 
 * Calculate color of rainbow spectrum corresponding to ratio between <value> and <max_N>
 * 
 * @param value The number for which the color is calculated
 * @param max_N The value for which white is returned
 * @param rr Where to store red value
 * @param gg Where to store green value
 * @param bb Where to store blue value
 * 
 * @return none
*/
void intToRainbowRGB(int value, int max_N, float& rr, float& gg, float& bb) {
    if (value == max_N) {
	rr = 1.0f; gg = 1.0f; bb = 1.0f;
	return;
    }
    value = std::max(0, std::min(value, max_N));

    float normalized = static_cast<float>(value) / max_N;

    // Break the rainbow into 6 segments
    float segmentSize = 1.0f / 6.0f;
    int segment = static_cast<int>(normalized / segmentSize);
    float segmentPosition = (normalized - segment * segmentSize) / segmentSize;

    // Interpolate between colors in each segment
    switch (segment) {
        case 0: // Red to Yellow
            rr = 1.0f;
            gg = segmentPosition;
            bb = 0.0f;
            break;
        case 1: // Yellow to Green
            rr = 1.0f - segmentPosition;
            gg = 1.0f;
            bb = 0.0f;
            break;
        case 2: // Green to Cyan
            rr = 0.0f;
            gg = 1.0f;
            bb = segmentPosition;
            break;
        case 3: // Cyan to Blue
            rr = 0.0f;
            gg = 1.0f - segmentPosition;
            bb = 1.0f;
            break;
        case 4: // Blue to Magenta
            rr = segmentPosition;
            gg = 0.0f;
            bb = 1.0f;
            break;
        case 5: // Magenta to Red
            rr = 1.0f;
            gg = 0.0f;
            bb = 1.0f - segmentPosition;
            break;
        default: // Should never happen
            rr = 0.0f;
            gg = 0.0f;
            bb = 0.0f;
            break;
    }
}
