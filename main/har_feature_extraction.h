#pragma once

#include <math.h>
#include <stddef.h>
#include "har_model_int8.h"

typedef struct {
    float ax;
    float ay;
    float az;
} har_accel_sample_t;

static inline float har_safe_sqrtf(float x) {
    return x > 0.0f ? sqrtf(x) : 0.0f;
}

static inline void har_extract_features_from_accel_window(const har_accel_sample_t window[HAR_WINDOW_SIZE], float features[HAR_NUM_FEATURES]) {
    float signals[4][HAR_WINDOW_SIZE];

    for (int i = 0; i < HAR_WINDOW_SIZE; ++i) {
        const float x = window[i].ax;
        const float y = window[i].ay;
        const float z = window[i].az;
        signals[0][i] = x;
        signals[1][i] = y;
        signals[2][i] = z;
        signals[3][i] = har_safe_sqrtf(x * x + y * y + z * z);
    }

    int k = 0;
    for (int c = 0; c < 4; ++c) {
        float sum = 0.0f;
        float sumsq = 0.0f;
        float sumabs = 0.0f;
        float minv = signals[c][0];
        float maxv = signals[c][0];

        for (int i = 0; i < HAR_WINDOW_SIZE; ++i) {
            const float v = signals[c][i];
            sum += v;
            sumsq += v * v;
            sumabs += fabsf(v);
            if (v < minv) minv = v;
            if (v > maxv) maxv = v;
        }

        const float mean = sum / (float)HAR_WINDOW_SIZE;
        float var = 0.0f;
        for (int i = 0; i < HAR_WINDOW_SIZE; ++i) {
            const float d = signals[c][i] - mean;
            var += d * d;
        }
        const float std = har_safe_sqrtf(var / (float)HAR_WINDOW_SIZE);
        const float rms = har_safe_sqrtf(sumsq / (float)HAR_WINDOW_SIZE);
        const float mean_abs = sumabs / (float)HAR_WINDOW_SIZE;
        const float range = maxv - minv;

        float diff_abs_sum = 0.0f;
        float diff_sum = 0.0f;
        float diffs[HAR_WINDOW_SIZE - 1];
        for (int i = 0; i < HAR_WINDOW_SIZE - 1; ++i) {
            const float d = signals[c][i + 1] - signals[c][i];
            diffs[i] = d;
            diff_abs_sum += fabsf(d);
            diff_sum += d;
        }
        const float diff_mean = diff_sum / (float)(HAR_WINDOW_SIZE - 1);
        float diff_var = 0.0f;
        for (int i = 0; i < HAR_WINDOW_SIZE - 1; ++i) {
            const float d = diffs[i] - diff_mean;
            diff_var += d * d;
        }
        const float mean_abs_diff = diff_abs_sum / (float)(HAR_WINDOW_SIZE - 1);
        const float std_diff = har_safe_sqrtf(diff_var / (float)(HAR_WINDOW_SIZE - 1));

        int zero_cross = 0;
        for (int i = 0; i < HAR_WINDOW_SIZE - 1; ++i) {
            const float a = signals[c][i] - mean;
            const float b = signals[c][i + 1] - mean;
            if ((a * b) < 0.0f) zero_cross++;
        }
        const float zero_cross_rate = (float)zero_cross / (float)(HAR_WINDOW_SIZE - 1);

        features[k++] = mean;
        features[k++] = std;
        features[k++] = minv;
        features[k++] = maxv;
        features[k++] = rms;
        features[k++] = mean_abs;
        features[k++] = range;
        features[k++] = mean_abs_diff;
        features[k++] = std_diff;
        features[k++] = zero_cross_rate;
    }

    for (int pair = 0; pair < 3; ++pair) {
        int a = 0, b = 1;
        if (pair == 1) { a = 0; b = 2; }
        if (pair == 2) { a = 1; b = 2; }

        float mean_a = 0.0f;
        float mean_b = 0.0f;
        for (int i = 0; i < HAR_WINDOW_SIZE; ++i) {
            mean_a += signals[a][i];
            mean_b += signals[b][i];
        }
        mean_a /= (float)HAR_WINDOW_SIZE;
        mean_b /= (float)HAR_WINDOW_SIZE;

        float cov = 0.0f;
        float var_a = 0.0f;
        float var_b = 0.0f;
        for (int i = 0; i < HAR_WINDOW_SIZE; ++i) {
            const float da = signals[a][i] - mean_a;
            const float db = signals[b][i] - mean_b;
            cov += da * db;
            var_a += da * da;
            var_b += db * db;
        }
        cov /= (float)HAR_WINDOW_SIZE;
        var_a /= (float)HAR_WINDOW_SIZE;
        var_b /= (float)HAR_WINDOW_SIZE;
        const float denom = (har_safe_sqrtf(var_a) + 1e-6f) * (har_safe_sqrtf(var_b) + 1e-6f);
        features[k++] = cov / denom;
    }
}
