#include <algorithm>

// Function written by Deepseek
void intToRainbowRGB(int value, int max_N, float& r, float& g, float& b) {
    if (value == max_N) {
	r = 1.0f; g = 1.0f; b = 1.0f;
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
            r = 1.0f;
            g = segmentPosition;
            b = 0.0f;
            break;
        case 1: // Yellow to Green
            r = 1.0f - segmentPosition;
            g = 1.0f;
            b = 0.0f;
            break;
        case 2: // Green to Cyan
            r = 0.0f;
            g = 1.0f;
            b = segmentPosition;
            break;
        case 3: // Cyan to Blue
            r = 0.0f;
            g = 1.0f - segmentPosition;
            b = 1.0f;
            break;
        case 4: // Blue to Magenta
            r = segmentPosition;
            g = 0.0f;
            b = 1.0f;
            break;
        case 5: // Magenta to Red
            r = 1.0f;
            g = 0.0f;
            b = 1.0f - segmentPosition;
            break;
        default: // Should never happen
            r = 0.0f;
            g = 0.0f;
            b = 0.0f;
            break;
    }
}
