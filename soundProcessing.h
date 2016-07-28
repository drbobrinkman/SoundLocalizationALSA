/** \file soundProcessing.h
 * Collection of functions for various kinds of sound processing.
 *
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-28
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

#include <vector>
#include <cstdint> //For int16_t

/*! Compute the mean and standard deviation of each channel of the sound
 *
 * \param buffer a buffer of sound data. Assumed to be 4 channels, interleaved
 * \return a vector of pairs, one per channel of the sound. First item in the
 *         pair is the mean of the signal, and the second is the standard
 *         deviation. We use the standard deviation as our measure of
 *         volume/loudness throughout.
 *
 * \note This is a single pass linear time algorithm
 */
std::vector<std::pair<float, float> >
meansAndStdDevs(const std::vector<int16_t>& buffer);

/*! Compute the normalized dot product of two channels of a sound, with an 
 *  optional 
 *  offset. Used in computing cross correlation (xcorr)
 *
 * \param buffer A sound sample, consisting of 4 channels, interleaved.
 * \param ch1 which channel number, 0-3, to use for first vector 
 * \param ch2 which channle number, 0-3, to use for second vector
 * \param offset to use
 *
 * \return Let \f$x_i\f$ represent the samples from the first channel, 
 * \f$y_i\f$
 * the 
 * samples of the second channel, \f$n\f$ the number of samples, 
 * and \f$o\f$ the offset. 
 * Assuming offset is positive, calculates 
 * \f$\frac{1}{n-o}\sum_{i=0}^{n-o-1} x_iy_{i+o}\f$. If offset is
 * negative, returns
 * \f$\frac{1}{n-o}\sum_{i=0}^{n-o-1} x_{i+o}y_{i}\f$
 */
float
dotWithOffset(const std::vector<int16_t>& buffer,
	      unsigned int ch1, unsigned int ch2,
	      int offset);

/*! Computes the similarity of two channels at various time shifts.
 *
 * \param buffer a sound clip, assumed to be 4 channels interleaved
 * \param ch1 which channel to use for first channel to compare
 * \param ch2 which channel to use for second channel to compare
 * \param range try all time lags between -range and range (inclusive)
 *
 * \return One pair for each integer between -range and range (inclusive).
 *         first item in each pair is an integer between -range and range,
 *         and the second item is the result of using that number as the
 *         offset in a call to dotWithOffset
 */
std::vector<std::pair<float, float> >
xcorr(const std::vector<int16_t>& buffer,
      unsigned int ch1, unsigned int ch2,
      int range);

/*! Estimates the delay between two signals based on the xcorr between them
 *
 * \param buffer a sound clip, assumed to be 4 channels interleaved
 * \param ch1 which channel (0-3) to use for first channel to compare
 * \param ch2 which channel (0-3) to use for second channel to compare
 * \param range Only test delays between -range and range (inclusive)
 *
 * \return first item is the delay between the two signals. The second
 * value is the ratio of how much of the energy of the two signals is
 * explainable by this offset. A value of 1.0 indicates a perfect alignment,
 * lower values indicate less perfect alignment.
 */
std::pair<float, float>
delay(const std::vector<int16_t>& buffer,
      unsigned int ch1, unsigned int ch2,
      int range);

/*! For each channel, shift it up or down so the mean becomes zero.
 *
 * \param buffer a sound clip, assumed to be 4 channels interleaved
 * \param stats the list of means and standard deviations for each channel,
 * as generated by meansAndStdDevs
 */
void
recenter(std::vector<int16_t>& buffer,
	 std::vector<std::pair<float, float> > stats);
