#pragma once
#include <Arduino.h>

// ============================================================
// Chi-Rho bitmap — 16×16 px, XBM format (LSB-first per byte).
//
// To swap the graphic:
//   1. Design a new 16×16 (or different size) bitmap.
//   2. Export as XBM from GIMP (File → Export As → .xbm).
//   3. Replace CHI_RHO_BITS with the exported array body.
//   4. Update CHI_RHO_W / CHI_RHO_H if the size changed.
//
// Used in StatusPage:
//   display.drawXBMP(x, y, CHI_RHO_W, CHI_RHO_H, CHI_RHO_BITS);
// ============================================================

// Visual layout (# = pixel ON, . = OFF):
//
//  Col: 0123456789ABCDEF
//  R0:  ....######......   P arc top
//  R1:  ...#......#.....
//  R2:  ..#........#....
//  R3:  ..#........#....
//  R4:  ...#......#.....
//  R5:  ....######......   P arc bottom
//  R6:  .......##.......   P stem
//  R7:  #......##......#   Chi arms begin
//  R8:  .#.....##.....#.
//  R9:  ..#....##....#..
//  R10: ...#...##...#...
//  R11: ....#..##..#....
//  R12: .....#.##.#.....
//  R13: ......####......   Chi centre
//  R14: .......##.......   stem
//  R15: ......####......   base flourish

static const uint8_t CHI_RHO_BITS[] PROGMEM = {
    0xF0, 0x03,  // Row  0
    0x08, 0x04,  // Row  1
    0x04, 0x08,  // Row  2
    0x04, 0x08,  // Row  3
    0x08, 0x04,  // Row  4
    0xF0, 0x03,  // Row  5
    0x80, 0x01,  // Row  6
    0x81, 0x81,  // Row  7
    0x82, 0x41,  // Row  8
    0x84, 0x21,  // Row  9
    0x88, 0x11,  // Row 10
    0x90, 0x09,  // Row 11
    0xA0, 0x05,  // Row 12
    0xC0, 0x03,  // Row 13
    0x80, 0x01,  // Row 14
    0xC0, 0x03,  // Row 15
};

static constexpr uint8_t CHI_RHO_W = 16;
static constexpr uint8_t CHI_RHO_H = 16;
