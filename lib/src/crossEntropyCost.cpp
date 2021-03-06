/****************************************************************************
** Copyright (c) 2017 Adrian Schneider
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
*****************************************************************************/

#include "crossEntropyCost.h"
#include "neuron.h"


CrossEntropyCost::CrossEntropyCost()
{

}

CrossEntropyCost::~CrossEntropyCost()
{

}

Eigen::MatrixXd CrossEntropyCost::delta(const Eigen::MatrixXd &/*z_weightdInput*/, const Eigen::MatrixXd &a_activation,
                                        const Eigen::MatrixXd &y_expected) const
{
    return a_activation - y_expected;
}

double CrossEntropyCost::cost(const Eigen::MatrixXd &a_activation, const Eigen::MatrixXd &y_expected) const
{
    Eigen::MatrixXd ones = Eigen::MatrixXd::Constant( y_expected.rows(), y_expected.cols(), 1.0 );

    Eigen::MatrixXd d =  - ((y_expected.array() * a_activation.array().log()) +  // y * ln(a)  +
                           (ones - y_expected).array() * (ones-a_activation).array().log()).matrix();

    Eigen::MatrixXd sums = d.colwise().sum();

    return sums.sum() / double(sums.cols());
}
