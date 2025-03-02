/*
 * Copyright (C) 2024 LibreMobileOS Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CameraProviderExtension.h"

#include <fstream>

#define TORCH_BRIGHTNESS "brightness"
#define TORCH_MAX_BRIGHTNESS "max_brightness"

static std::string kTorchLedPaths[] = {
        "/sys/devices/platform/soc/c440000.qcom,spmi/spmi-0/spmi0-03/c440000.qcom,spmi:qcom,pmi8998@3:qcom,leds@d300/leds/flashlight",
};

/**
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value << std::endl;
}

/**
 * Read value from the path and close file.
 */
template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

bool supportsTorchStrengthControlExt() {
    return true;
}

bool supportsSetTorchModeExt() {
    return true;
}

int32_t getTorchDefaultStrengthLevelExt() {
    return 80;
}

int32_t getTorchMaxStrengthLevelExt() {
    // 200 (out of 500) is a sane max brightness
    return 200;
}

int32_t getTorchStrengthLevelExt() {
    // We write same value in the both LEDs,
    // so get from one.
    auto node = kTorchLedPaths[0] + "/" + TORCH_BRIGHTNESS;
    return get(node, 0);
}

void setTorchStrengthLevelExt(int32_t torchStrength, bool enabled) {
    if (!enabled) {
        torchStrength = 0;
    }
    for (auto& path : kTorchLedPaths) {
        auto node = path + "/" + TORCH_BRIGHTNESS;
        // The Xiaomi PMI8998 kernel driver ignores brightness updates 
        // if the switch is already ON. We must force it OFF first to apply the new current!
        if (enabled && torchStrength > 0) {
            set(node, 0);
        }
        set(node, torchStrength);
    }
}

void setTorchModeExt(bool enabled) {
    int32_t strength = getTorchDefaultStrengthLevelExt();
    setTorchStrengthLevelExt(enabled ? strength : 0, enabled);
}
