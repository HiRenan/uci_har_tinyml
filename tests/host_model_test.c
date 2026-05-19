#include <stdio.h>
#include <math.h>
#include "../main/har_inference.h"
#include "expected_cases.h"

int main(void) {
    int failures = 0;
    for (int i = 0; i < HAR_EXPECTED_CASES_COUNT; ++i) {
        float logits[HAR_NUM_CLASSES];
        int pred = har_predict_from_features(HAR_EXPECTED_CASES[i].f, logits);
        printf("case=%d expected=%s predicted=%s logits=[", i,
               HAR_CLASS_NAMES[HAR_EXPECTED_CASES[i].expected], HAR_CLASS_NAMES[pred]);
        for (int c = 0; c < HAR_NUM_CLASSES; ++c) {
            printf("%s%.5f", c == 0 ? "" : ",", logits[c]);
        }
        printf("]\n");
        if (pred != HAR_EXPECTED_CASES[i].expected) {
            failures++;
        }
    }
    if (failures != 0) {
        printf("FAIL: %d caso(s) falharam.\n", failures);
        return 1;
    }
    printf("OK: todos os casos de teste do modelo UCI HAR passaram.\n");
    return 0;
}
