#!/usr/bin/env python3
"""
Treina o modelo TinyML do projeto a partir do dataset publico UCI HAR.

Uso:
  python training/train_uci_har_accel_model.py --download

O script baixa o zip oficial da UCI, extrai os sinais total_acc_x/y/z,
calcula 43 features por janela de 128 amostras e exporta o modelo compactado
para main/har_model_int8.h.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import urllib.request
import zipfile
from pathlib import Path

import numpy as np
import pandas as pd
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.neural_network import MLPClassifier
from sklearn.preprocessing import StandardScaler

UCI_URL = "https://archive.ics.uci.edu/static/public/240/human+activity+recognition+using+smartphones.zip"
CLASS_NAMES = ["WALKING", "WALKING_UPSTAIRS", "WALKING_DOWNSTAIRS", "SITTING", "STANDING", "LAYING"]
BASE_FEATURES = ["mean", "std", "min", "max", "rms", "mean_abs", "range", "mean_abs_diff", "std_diff", "zero_cross_rate"]
FEATURE_NAMES = [f"{ch}_{feat}" for ch in ["acc_x_g", "acc_y_g", "acc_z_g", "acc_mag_g"] for feat in BASE_FEATURES] + [
    "corr_acc_x_y", "corr_acc_x_z", "corr_acc_y_z"
]
ROOT = Path(__file__).resolve().parents[1]
DATASET_DIR = ROOT / "dataset"
RAW_DIR = DATASET_DIR / "raw_uci_har"


def download_and_extract() -> Path:
    DATASET_DIR.mkdir(parents=True, exist_ok=True)
    zip_path = DATASET_DIR / "uci_har_official.zip"
    if not zip_path.exists():
        print(f"Baixando dataset oficial: {UCI_URL}")
        urllib.request.urlretrieve(UCI_URL, zip_path)
    if RAW_DIR.exists():
        shutil.rmtree(RAW_DIR)
    RAW_DIR.mkdir(parents=True)
    print("Extraindo dataset...")
    with zipfile.ZipFile(zip_path, "r") as zf:
        zf.extractall(RAW_DIR)
    return RAW_DIR / "UCI HAR Dataset"


def dataset_root_from_args(args: argparse.Namespace) -> Path:
    if args.download:
        return download_and_extract()
    if args.dataset_root:
        return Path(args.dataset_root)
    candidate = RAW_DIR / "UCI HAR Dataset"
    if candidate.exists():
        return candidate
    raise SystemExit("Dataset bruto nao encontrado. Use --download ou --dataset-root /caminho/UCI HAR Dataset")


def load_raw(dataset_root: Path, split: str):
    inertial = dataset_root / split / "Inertial Signals"
    xs = []
    for axis in "xyz":
        xs.append(np.loadtxt(inertial / f"total_acc_{axis}_{split}.txt"))
    X = np.stack(xs, axis=2).astype(np.float32)
    y = np.loadtxt(dataset_root / split / f"y_{split}.txt", dtype=int) - 1
    return X, y




def load_processed_features():
    train_path = DATASET_DIR / "uci_har_accel_features_train.csv"
    test_path = DATASET_DIR / "uci_har_accel_features_test.csv"
    if not train_path.exists() or not test_path.exists():
        raise SystemExit("CSV processado nao encontrado. Use --download ou informe --dataset-root.")
    train_df = pd.read_csv(train_path)
    test_df = pd.read_csv(test_path)
    Xtr = train_df[FEATURE_NAMES].to_numpy(dtype=np.float32)
    Xte = test_df[FEATURE_NAMES].to_numpy(dtype=np.float32)
    ytr = train_df["label_id"].to_numpy(dtype=int) - 1
    yte = test_df["label_id"].to_numpy(dtype=int) - 1
    return Xtr, ytr, Xte, yte


def extract_features(X: np.ndarray) -> np.ndarray:
    mag = np.sqrt(np.sum(X * X, axis=2, keepdims=True))
    data = np.concatenate([X, mag], axis=2)
    feats = []
    for c in range(4):
        s = data[:, :, c]
        mean = s.mean(axis=1)
        std = s.std(axis=1)
        minv = s.min(axis=1)
        maxv = s.max(axis=1)
        rms = np.sqrt(np.mean(s * s, axis=1))
        meanabs = np.mean(np.abs(s), axis=1)
        rangev = maxv - minv
        diff = np.diff(s, axis=1)
        madiff = np.mean(np.abs(diff), axis=1)
        stddiff = diff.std(axis=1)
        centered = s - mean[:, None]
        zc = np.sum((centered[:, :-1] * centered[:, 1:]) < 0, axis=1) / 127.0
        feats.extend([mean, std, minv, maxv, rms, meanabs, rangev, madiff, stddiff, zc])
    for a, b in [(0, 1), (0, 2), (1, 2)]:
        xa = X[:, :, a]
        xb = X[:, :, b]
        ma = xa.mean(axis=1)
        mb = xb.mean(axis=1)
        sa = xa.std(axis=1) + 1e-6
        sb = xb.std(axis=1) + 1e-6
        corr = np.mean((xa - ma[:, None]) * (xb - mb[:, None]), axis=1) / (sa * sb)
        feats.append(corr)
    return np.vstack(feats).T.astype(np.float32)


def quantize_symmetric(W: np.ndarray):
    maxabs = float(np.max(np.abs(W)))
    scale = maxabs / 127.0 if maxabs > 0 else 1.0
    Q = np.clip(np.round(W / scale), -127, 127).astype(np.int8)
    return Q, scale


def write_header(path: Path, scaler, clf, W1q, W2q, W1scale, W2scale):
    def c_float_array(name, arr, per_line=6):
        vals = [f"{float(x): .9g}f" for x in np.array(arr, dtype=np.float32).ravel()]
        lines = ["    " + ", ".join(vals[i:i + per_line]) for i in range(0, len(vals), per_line)]
        return f"static const float {name}[{len(vals)}] = {{\n" + ",\n".join(lines) + "\n};\n"

    def c_int8_2d(name, arr):
        rows = ["    {" + ", ".join(str(int(x)) for x in row) + "}" for row in arr]
        return f"static const int8_t {name}[{arr.shape[0]}][{arr.shape[1]}] = {{\n" + ",\n".join(rows) + "\n};\n"

    def c_string_array(name, values):
        return f"static const char *const {name}[{len(values)}] = {{" + ", ".join(f'\"{v}\"' for v in values) + "};\n"

    W1 = clf.coefs_[0]
    b1 = clf.intercepts_[0]
    b2 = clf.intercepts_[1]
    text = f"""#pragma once

#include <stdint.h>

#define HAR_NUM_FEATURES {W1.shape[0]}
#define HAR_HIDDEN_UNITS {W1.shape[1]}
#define HAR_NUM_CLASSES {len(CLASS_NAMES)}
#define HAR_WINDOW_SIZE 128
#define HAR_SAMPLE_RATE_HZ 50

// Gerado automaticamente por training/train_uci_har_accel_model.py.

{c_string_array('HAR_CLASS_NAMES', CLASS_NAMES)}
{c_string_array('HAR_FEATURE_NAMES', FEATURE_NAMES)}
{c_float_array('HAR_FEATURE_MEAN', scaler.mean_)}
{c_float_array('HAR_FEATURE_SCALE', scaler.scale_)}
static const float HAR_W1_SCALE = {W1scale:.12g}f;
static const float HAR_W2_SCALE = {W2scale:.12g}f;
{c_float_array('HAR_B1', b1)}
{c_float_array('HAR_B2', b2)}
{c_int8_2d('HAR_W1_Q', W1q)}
{c_int8_2d('HAR_W2_Q', W2q)}
"""
    path.write_text(text, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--download", action="store_true", help="Baixa e extrai o dataset oficial da UCI")
    parser.add_argument("--dataset-root", default=None, help="Caminho para a pasta 'UCI HAR Dataset'")
    parser.add_argument("--from-processed", action="store_true", help="Reusa os CSVs de features ja incluidos em dataset/")
    args = parser.parse_args()

    if args.from_processed:
        print("Reutilizando CSVs de features ja incluidos em dataset/ ...")
        Xtr, ytr, Xte, yte = load_processed_features()
    else:
        dataset_root = dataset_root_from_args(args)
        Xtr_raw, ytr = load_raw(dataset_root, "train")
        Xte_raw, yte = load_raw(dataset_root, "test")
        Xtr = extract_features(Xtr_raw)
        Xte = extract_features(Xte_raw)

        train_df = pd.DataFrame(Xtr, columns=FEATURE_NAMES)
        train_df.insert(0, "label_id", ytr + 1)
        train_df.insert(1, "label_name", [CLASS_NAMES[i] for i in ytr])
        train_df.to_csv(DATASET_DIR / "uci_har_accel_features_train.csv", index=False)

        test_df = pd.DataFrame(Xte, columns=FEATURE_NAMES)
        test_df.insert(0, "label_id", yte + 1)
        test_df.insert(1, "label_name", [CLASS_NAMES[i] for i in yte])
        test_df.to_csv(DATASET_DIR / "uci_har_accel_features_test.csv", index=False)

    scaler = StandardScaler().fit(Xtr)
    Xtr_s = scaler.transform(Xtr)
    Xte_s = scaler.transform(Xte)
    clf = MLPClassifier(hidden_layer_sizes=(32,), activation="relu", solver="adam", alpha=1e-4,
                        learning_rate_init=0.002, max_iter=250, random_state=42,
                        early_stopping=True, validation_fraction=0.15, n_iter_no_change=15)
    clf.fit(Xtr_s, ytr)

    W1q, W1scale = quantize_symmetric(clf.coefs_[0].astype(np.float32))
    W2q, W2scale = quantize_symmetric(clf.coefs_[1].astype(np.float32))

    def predict_quantized(Xfeat):
        Xn = (Xfeat - scaler.mean_.astype(np.float32)) / scaler.scale_.astype(np.float32)
        H = np.maximum(0.0, Xn @ (W1q.astype(np.float32) * W1scale) + clf.intercepts_[0].astype(np.float32))
        logits = H @ (W2q.astype(np.float32) * W2scale) + clf.intercepts_[1].astype(np.float32)
        return np.argmax(logits, axis=1)

    pred_float = clf.predict(Xte_s)
    pred_q = predict_quantized(Xte)
    report = {
        "dataset": "UCI Human Activity Recognition Using Smartphones",
        "source_url": "https://archive.ics.uci.edu/dataset/240/human+activity+recognition+using+smartphones",
        "samples_train": int(Xtr.shape[0]),
        "samples_test": int(Xte.shape[0]),
        "features_per_window": int(Xtr.shape[1]),
        "float_test_accuracy": float(accuracy_score(yte, pred_float)),
        "quantized_test_accuracy": float(accuracy_score(yte, pred_q)),
        "classes": CLASS_NAMES,
        "model": "MLP 43-32-6, ReLU, pesos int8"
    }
    (ROOT / "training" / "model_report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    pd.DataFrame(confusion_matrix(yte, pred_q), index=CLASS_NAMES, columns=CLASS_NAMES).to_csv(ROOT / "training" / "confusion_matrix_quantized_test.csv")
    (ROOT / "training" / "classification_report_test.txt").write_text(classification_report(yte, pred_q, target_names=CLASS_NAMES), encoding="utf-8")
    write_header(ROOT / "main" / "har_model_int8.h", scaler, clf, W1q, W2q, W1scale, W2scale)
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
