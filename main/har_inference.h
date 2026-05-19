#pragma once

#include <math.h>
#include <stdint.h>
#include "har_model_int8.h"

static inline int har_argmax6(const float v[HAR_NUM_CLASSES]) {
    int best = 0;
    float best_v = v[0];
    for (int i = 1; i < HAR_NUM_CLASSES; ++i) {
        if (v[i] > best_v) {
            best_v = v[i];
            best = i;
        }
    }
    return best;
}

static inline int har_predict_from_features(const float features[HAR_NUM_FEATURES], float logits[HAR_NUM_CLASSES]) {
    float hidden[HAR_HIDDEN_UNITS];

    for (int h = 0; h < HAR_HIDDEN_UNITS; ++h) {
        float acc = HAR_B1[h];
        for (int i = 0; i < HAR_NUM_FEATURES; ++i) {
            float x = (features[i] - HAR_FEATURE_MEAN[i]) / HAR_FEATURE_SCALE[i];
            acc += x * ((float)HAR_W1_Q[i][h] * HAR_W1_SCALE);
        }
        hidden[h] = acc > 0.0f ? acc : 0.0f;
    }

    for (int c = 0; c < HAR_NUM_CLASSES; ++c) {
        float acc = HAR_B2[c];
        for (int h = 0; h < HAR_HIDDEN_UNITS; ++h) {
            acc += hidden[h] * ((float)HAR_W2_Q[h][c] * HAR_W2_SCALE);
        }
        logits[c] = acc;
    }

    return har_argmax6(logits);
}
