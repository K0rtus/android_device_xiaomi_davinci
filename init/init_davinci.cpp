/*
   Copyright (C) 2020 The LineageOS Project.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include <unistd.h>
#include <vector>

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;

std::vector<std::string> ro_props_default_source_order = {
    "",
    "bootimage.",
    "odm.",
    "product.",
    "system.",
    "vendor.",
};

void property_override(char const prop[], char const value[]) {
    prop_info *pi = (prop_info *) __system_property_find(prop);

    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void set_ro_prop(const std::string &prop_type, const std::string &prop, const std::string &value) {
    for (const auto &source : ro_props_default_source_order) {
    	if (prop_type == "build") {
            auto prop_name = "ro." + source + "build." + prop;
            if (source == "")
                property_override(prop_name.c_str(), value.c_str());
            else
                property_override(prop_name.c_str(), value.c_str());
        }
    	if (prop_type == "product") {
            auto prop_name = "ro.product." + source + prop;
            property_override(prop_name.c_str(), value.c_str());
        }
    }
}

void vendor_load_properties() {
    std::string region = GetProperty("ro.boot.hwc", "GLOBAL");

    std::string model;
    std::string device;
    std::string fingerprint;
    std::string description;
    std::string mod_device;

    device = "davinci";
    model = "Redmi K20";

    if (region == "GLOBAL") {
        model = "Mi 9T";
        mod_device = "davinci_global";
    } else if (region == "INDIA") {
        device = "davinciin";
        mod_device = "davinciin_in_global";
    }

    // SafetyNet workaround
    property_override("ro.boot.verifiedbootstate", "green");
    fingerprint = "Xiaomi/dipper/dipper:8.1.0/OPM1.171019.011/V9.5.5.0.OEAMIFA:user/release-keys";
    description = "dipper-user 8.1.0 OPM1.171019.011 V9.5.5.0.OEAMIFA release-keys";

    set_ro_prop("build","fingerprint", fingerprint);
    set_ro_prop("product","device", device);
    set_ro_prop("product","model", model);
    property_override("ro.build.description", description.c_str());
    if (mod_device != "") {
        property_override("ro.product.mod_device", mod_device.c_str());
    }

#if !defined(__ANDROID_RECOVERY__)
  if ((GetProperty("ro.build.type", "userdebug") != "eng") &&
      (GetProperty("ro.override_debuggable", "1") != "0")) {
        property_override("ro.debuggable", "0");
    }
#endif
}
