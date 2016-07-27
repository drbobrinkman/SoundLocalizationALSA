/** \file constants.h
 * Collection of constants that are used in multiple cpp files
 *
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-27
 */

/*
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

/*! Real world distance between micropohes, which should be arranged
 *  in a tetrahedron */
//Used here and locationlut.cpp
constexpr float SENSOR_SPACING_INCHES = 7.0f;

/*! Conversion rate from inches to meters */
//Used here and locationlut.cpp
constexpr float METERS_PER_INCH = 0.0254f;

/*! Sample rate of the microphone we use 
    \bug Should get this from the Microphone class, instead of hard coding it
*/
//Used here and locationlut.cpp
constexpr unsigned int SAMPLES_PER_SECOND = 16000;

/*! Number of channels coming from microphone 
    \bug Should get this from the Microphone class, instead of hard coding it
*/
constexpr unsigned int NUM_CHANNELS = 4;

/*! Speed of sound in seconds per meter*/
//Used here and locationlut.cpp
constexpr float SPEED_OF_SOUND_SECONDS_PER_METER = (float)(1.0/340.29);

/*! Speed of sound in seconds per inch */
//Used only here
constexpr float SPEED_OF_SOUND_SECONDS_PER_INCH = METERS_PER_INCH *
  SPEED_OF_SOUND_SECONDS_PER_METER;

/*! Sensor spacing in terms of how many microphone samples elapse as
 *  sound travels from one microphone to another */
//Used in main.cpp and server.cpp
constexpr float SENSOR_SPACING_SAMPLES = SENSOR_SPACING_INCHES *
  SAMPLES_PER_SECOND * SPEED_OF_SOUND_SECONDS_PER_INCH;

/*! Signals less loud than this are considered silence. Determine
 *  empirically based on application */
//Used in server.cpp and tracker.cpp
constexpr float SILENCE_LOUDNESS = 250.0f;

/*! Number of times we sample the microphone per second. */
//Used in tracker.cpp and microphone.cpp
constexpr float TARGET_FRAME_RATE = 15.0f;
