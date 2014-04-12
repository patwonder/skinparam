
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of fork of pbrt (pbrt-v2-skin).

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include "stdafx.h"
#include "skincoeffs.h"

using namespace ProfileFit;

const float SkinCoefficients::ohg_lambdas[] = {
	400, 405, 410, 415, 420, 425, 430, 435, 440, 445,
	450, 455, 460, 465, 470, 475, 480, 485, 490, 495,
	500, 505, 510, 515, 520, 525, 530, 535, 540, 545,
	550, 555, 560, 565, 570, 575, 580, 585, 590, 595,
	600, 605, 610, 615, 620, 625, 630, 635, 640, 645,
	650, 655, 660, 665, 670, 675, 680, 685, 690, 695,
	700
};
const float SkinCoefficients::ohg_vals[] = {
	266200, 331450, 466800, 523100, 480400, 351100, 246100, 149050,
	102600, 78880, 62820, 51525, 44480, 38440, 33210, 29480, 26630,
	24925, 23680, 22155, 20930, 20185, 20040, 20715, 24200, 30885,
	39960, 48335, 53240, 50985, 43020, 35650, 32610, 35210, 44500,
	54425, 50100, 30620, 14400, 6681.5, 3200, 1958.5, 1506, 1166.5,
	942, 740.8, 610, 495.6, 442, 397.7, 368, 340.3, 319.6, 305.6,
	294, 283.8, 277.6, 273.6, 276, 280.6, 290
};
const int SkinCoefficients::ohg_n = ARRAYSIZE(ohg_lambdas);

const float SkinCoefficients::dhg_lambdas[] = {
	400, 405, 410, 415, 420, 425, 430, 435, 440, 445,
	450, 455, 460, 465, 470, 475, 480, 485, 490, 495,
	500, 505, 510, 515, 520, 525, 530, 535, 540, 545,
	550, 555, 560, 565, 570, 575, 580, 585, 590, 595,
	600, 605, 610, 615, 620, 625, 630, 635, 640, 645,
	650, 655, 660, 665, 670, 675, 680, 685, 690, 695,
	700
};
const float SkinCoefficients::dhg_vals[] = {
	223300, 261950, 304000, 353200, 407600, 471500, 528600, 549600,
	413300, 259950, 103300, 33435, 23390, 18700, 16160, 14920, 14550,
	15375, 16680, 18650, 20860, 23285, 25770, 28680, 31590, 35170,
	39040, 42840, 46590, 50490, 53410, 54530, 53790, 49700, 45070,
	40905, 37020, 33590, 28320, 21185, 14680, 12040, 9444, 7553.5,
	6510, 5763.5, 5149, 4666.5, 4345, 4026.5, 3750, 3481.5, 3227,
	3011, 2795, 2591, 2408, 2224.5, 2052, 1923.5, 1794
};
const int SkinCoefficients::dhg_n = ARRAYSIZE(dhg_lambdas);

namespace ProfileFit {

const float WLD_lambdas[] = {
	400, 405, 410, 415, 420, 425, 430, 435, 440, 445,
	450, 455, 460, 465, 470, 475, 480, 485, 490, 495,
	500, 505, 510, 515, 520, 525, 530, 535, 540, 545,
	550, 555, 560, 565, 570, 575, 580, 585, 590, 595,
	600, 605, 610, 615, 620, 625, 630, 635, 640, 645,
	650, 655, 660, 665, 670, 675, 680, 685, 690, 695,
	700
};

} // namespace ProfileFit
