#include "ec_adapter.h"

#include <windows.h>
#include <xinput.h>

static const char* k_id   = "xbox360-gamepad";
static const char* k_name = "Xbox 360 手柄";

/* XInput 只能报告 4 档电量，这里映射为百分比 */
static int level_to_percent(BYTE level) {
    switch (level) {
        case BATTERY_LEVEL_EMPTY:  return 0;
        case BATTERY_LEVEL_LOW:    return 25;
        case BATTERY_LEVEL_MEDIUM: return 60;
        case BATTERY_LEVEL_FULL:   return 100;
        default:                   return -1;
    }
}

EC_EXPORT const char* ec_adapter_id(void) {
    return k_id;
}

EC_EXPORT const char* ec_adapter_name(void) {
    return k_name;
}

EC_EXPORT int32_t ec_get_snapshot(struct ec_snapshot* 传出) {
    if (传出 == nullptr || 传出->struct_size < sizeof(struct ec_snapshot)) { return -1; }

    /* 依次探测 1~4 号手柄，取第一个带电池的（有线手柄报 DISCONNECTED，跳过） */
    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
        XINPUT_BATTERY_INFORMATION bat{};
        if (XInputGetBatteryInformation(i, BATTERY_DEVTYPE_GAMEPAD, &bat) != ERROR_SUCCESS) {
            continue;
        }
        if (bat.BatteryType != BATTERY_TYPE_ALKALINE && bat.BatteryType != BATTERY_TYPE_NIMH) {
            continue; /* 有线 / 未知类型，无电池数据 */
        }

        const int percent = level_to_percent(bat.BatteryLevel);
        if (percent < 0) { continue; }

        传出->has_battery = 1;
        传出->percent     = percent;
        传出->ac_online   = 0;          /* XInput 不提供充电/外接电源状态 */
        传出->charging    = 0;

        传出->remaining_wh       = NAN; /* 手柄电量不按 Wh 计，均为未知 */
        传出->full_wh            = NAN;
        传出->rate_watts         = NAN;
        传出->design_capacity_wh = NAN;
        传出->remaining_seconds  = -1;
        return 0;
    }
    return 1; /* 没有可报告电量的手柄 */
}
