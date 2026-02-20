#pragma once

// Masks are defined in RGB order
constexpr unsigned int MASK_SHADOW = 0x007F00;
constexpr unsigned int MASK_EDGE = 0x00C800;
constexpr unsigned int MASK_SHADOW_EDGE = 0x1FFF1F;
constexpr unsigned int MASK_BODY = 0x640000;
constexpr unsigned int MASK_LEGS = 0x780000;
constexpr unsigned int MASK_HEAD = 0x8C0000;
constexpr unsigned int MASK_BELT = 0xA00000;
constexpr unsigned int MASK_BOOTS = 0xB40000;
constexpr unsigned int MASK_WEAPON = 0xC80000;
constexpr unsigned int MASK_TRANSPARENT = 0xFFFFFF;
