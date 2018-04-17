/*
 * ESP8266 + FastLED + IR Remote + MSGEQ7: https://github.com/jasoncoon/esp8266-fastled-webserver
 * Copyright (C) 2015 Jason Coon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

enum class InputCommand {
  None,
  Up,
  Down,
  Left,
  Right,
  Select,
  Brightness,
  PlayMode,
  Palette,
  Power,
  BrightnessUp,
  BrightnessDown,

  Pattern1,
  Pattern2,
  Pattern3,
  Pattern4,
  Pattern5,
  Pattern6,
  Pattern7,
  Pattern8,
  Pattern9,
  Pattern10,
  Pattern11,
  Pattern12,

  RedUp,
  RedDown,
  GreenUp,
  GreenDown,
  BlueUp,
  BlueDown,

  Red,
  RedOrange,
  Orange,
  YellowOrange,
  Yellow,

  Green,
  Lime,
  Aqua,
  Teal,
  Navy,

  Blue,
  RoyalBlue,
  Purple,
  Indigo,
  Magenta,

  White,
  Pink,
  LightPink,
  BabyBlue,
  LightBlue,
};