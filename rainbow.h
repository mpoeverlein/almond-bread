#include <algorithm>
#include <vector>
#include <array>
#include <stdio.h>
#include <iostream>

// function mostly written by DeepSeek AI
void intToInferno(int value, int max_N, float& rr, float& gg, float& bb) {
    value = std::max(0, std::min(value, max_N));
    float normalized = static_cast<float>(value) / max_N; 

    // Inferno colormap data (approximated from matplotlib)
    // This uses a piecewise linear interpolation of key points
    if (normalized <= 0.0) {
        rr = 0; gg = 0; bb = 0;
        return;
    } else if (normalized >= 1.0) {
        rr = 252/255; gg = 255/255, bb = 164.0/255.0;   // Yellow-white at 1.0
        std::cout << bb << "\n";
        return;
    }

    std::cout << normalized << " ";
    
    // Key points in the inferno colormap (value, R, G, B)
    std::array<std::array<double, 4>, 9> inferno_points = {{
        {0.0,    0.0,    0.0,    0.0},
        {0.13,   27.0,   11.0,   120.0},
        {0.25,   81.0,   18.0,   123.0},
        {0.38,   134.0,  22.0,   110.0},
        {0.5,    185.0,  39.0,   88.0},
        {0.63,   225.0,  69.0,   41.0},
        {0.75,   243.0,  114.0,  22.0},
        {0.88,   252.0,  193.0,  50.0},
        {1.00,   252.0,  255.0,  164.0}
    }};

    // Find which segment our value falls into
    size_t segment = 0;
    for (size_t i = 0; i < inferno_points.size() - 2; ++i) {
        if (normalized >= inferno_points[i][0] && normalized < inferno_points[i+1][0]) {
            segment = i;
            break;
        }
    }
    if (normalized > inferno_points[inferno_points.size()-2][0]) {
        segment = inferno_points.size()-1;
    }
    
    // Linear interpolation within the segment
    double t = (normalized - inferno_points[segment][0]) / 
               (inferno_points[segment+1][0] - inferno_points[segment][0]);
    
    std::vector<float> color(3);
    for (int i = 0; i < 3; ++i) {
        float channel = inferno_points[segment][i+1] + 
                        t * (inferno_points[segment+1][i+1] - inferno_points[segment][i+1]);
        color[i] = std::round(channel);
        std::cout << color[i]/256 << " ";
    }
    std::cout << "\n";
    rr = color[0]/256;
    gg = color[1]/256;
    bb = color[2]/256;
    
}

void intToBWRGB(int value, int max_N, float& rr, float& gg, float& bb) {
    if (value == max_N) {
        rr = 1.0f; gg = 1.0f; bb = 1.0f;
        return;
    }
    value = std::max(0, std::min(value, max_N));
    float normalized = static_cast<float>(value) / max_N;
    if (normalized < 0.1) {
        normalized = 0.0f;
    } else {
        normalized = 1.0f;
    }
    rr = normalized;
    gg = normalized;
    bb = normalized;
}

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
