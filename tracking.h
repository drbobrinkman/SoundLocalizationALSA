/**
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **/

#pragma once

constexpr float SENSOR_SPACING_INCHES = 7.0f;
constexpr int SAMPLES_PER_SECOND = 16000;

constexpr float SPEED_OF_SOUND_SECONDS_PER_METER = (float)(1.0/340.29);
constexpr float METERS_PER_INCH = 0.0254f;
constexpr float SENSOR_SPACING_METERS = SENSOR_SPACING_INCHES*METERS_PER_INCH;
constexpr float SPEED_OF_SOUND_SECONDS_PER_INCH = METERS_PER_INCH *
  SPEED_OF_SOUND_SECONDS_PER_METER;

constexpr float SENSOR_SPACING_SAMPLES = SENSOR_SPACING_INCHES *
  SAMPLES_PER_SECOND * SPEED_OF_SOUND_SECONDS_PER_INCH;

constexpr float SPEED_OF_SOUND_SAMPLES_PER_METER = SAMPLES_PER_SECOND *
  SPEED_OF_SOUND_SECONDS_PER_METER;

constexpr int MAX_OFFSET = (int)(SENSOR_SPACING_SAMPLES*1.25);
constexpr int MIN_OFFSET = -MAX_OFFSET;
