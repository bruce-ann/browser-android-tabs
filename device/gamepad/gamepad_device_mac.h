// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_GAMEPAD_GAMEPAD_DEVICE_MAC_H_
#define DEVICE_GAMEPAD_GAMEPAD_DEVICE_MAC_H_

#include <CoreFoundation/CoreFoundation.h>
#include <ForceFeedback/ForceFeedback.h>
#include <IOKit/hid/IOHIDManager.h>
#include <stddef.h>

#include "device/gamepad/abstract_haptic_gamepad.h"
#include "device/gamepad/public/cpp/gamepad.h"

namespace device {

// GamepadDeviceMac represents a single gamepad device. Gamepad enumeration
// and state polling is handled through the raw HID interface, while haptics
// commands are issued through the ForceFeedback framework.
class GamepadDeviceMac : public AbstractHapticGamepad {
 public:
  GamepadDeviceMac(int location_id, IOHIDDeviceRef device_ref);
  ~GamepadDeviceMac() override;

  // Initialize |gamepad| with the number of buttons and axes described in the
  // device's elements array.
  bool AddButtonsAndAxes(Gamepad* gamepad);

  // Update the button and axis state in |gamepad| with the new data in |value|.
  // If the updated element is an axis, the axis value will first be normalized.
  void UpdateGamepadForValue(IOHIDValueRef value, Gamepad* gamepad);

  // Return the OS-assigned ID for this device.
  int GetLocationId() { return location_id_; }

  // Return true if |device| refers to this device.
  bool IsSameDevice(IOHIDDeviceRef device) { return device == device_ref_; }

  // Return true if this device supports force feedback through the
  // ForceFeedback framework.
  bool SupportsForceFeedback() { return ff_device_ref_ != nullptr; }

  // Starts vibrating the device with the specified magnitudes.
  void SetVibration(double strong_magnitude, double weak_magnitude) override;

  // Stop vibration by canceling any ongoing vibration effect.
  void SetZeroVibration() override;

 private:
  // Stop vibration and release held resources.
  void DoShutdown() override;

  // Return true if this element has a parent collection with a usage page that
  // suggests it could be a gamepad.
  static bool CheckCollection(IOHIDElementRef element);

  // Create a force feedback device node for controlling haptics on
  // |device_ref|. Ownership of the returned reference is retained by the
  // caller.
  static FFDeviceObjectReference CreateForceFeedbackDevice(
      IOHIDDeviceRef device_ref);

  // Create a force feedback effect on |ff_device_ref| and store the description
  // to |ff_effect|. Ownership of the returned reference is retained by the
  // caller.
  static FFEffectObjectReference CreateForceFeedbackEffect(
      FFDeviceObjectReference ff_device_ref,
      FFEFFECT* ff_effect,
      FFCUSTOMFORCE* ff_custom_force,
      LONG* force_data,
      DWORD* axes_data,
      LONG* direction_data);

  int location_id_;
  IOHIDDeviceRef device_ref_;

  IOHIDElementRef button_elements_[Gamepad::kButtonsLengthCap];
  IOHIDElementRef axis_elements_[Gamepad::kAxesLengthCap];
  CFIndex axis_minimums_[Gamepad::kAxesLengthCap];
  CFIndex axis_maximums_[Gamepad::kAxesLengthCap];
  CFIndex axis_report_sizes_[Gamepad::kAxesLengthCap];

  // Force feedback
  FFDeviceObjectReference ff_device_ref_;
  FFEffectObjectReference ff_effect_ref_;
  FFEFFECT ff_effect_;
  FFCUSTOMFORCE ff_custom_force_;
  LONG force_data_[2];
  DWORD axes_data_[2];
  LONG direction_data_[2];
};

}  // namespace device

#endif  // DEVICE_GAMEPAD_GAMEPAD_DEVICE_MAC_H_
