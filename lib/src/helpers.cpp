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

#include <Eigen/Dense>
#include <iostream>
#include <limits>
#include "helpers.h"

using namespace std;

void Helpers::printVector( const Eigen::VectorXd& vector, const string& name )
{
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
    cout << name << ":" << endl << vector.format(CleanFmt) << endl;
}

void Helpers::printMatrix( const Eigen::MatrixXd& mat, const string& name )
{
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
    cout << name << ":" << endl << mat.format(CleanFmt) << endl;
}

void Helpers::maxElement( const Eigen::MatrixXd& mat, unsigned long& m_idx, unsigned long& n_idx, double& maxVal)
{
    maxVal = numeric_limits<double>::min();
    for( unsigned int m = 0; m < mat.rows(); m++ )
    {
        for( unsigned int n = 0; n < mat.cols(); n++ )
        {
            double val = mat(m,n);
            if( val > maxVal )
            {
                maxVal = val;
                m_idx = m; n_idx = n;
            }
        }
    }
}

Eigen::MatrixXd Helpers::mean( const std::vector<Eigen::MatrixXd>& input )
{
    size_t n = input.size();
    if( n == 0 )
    {
        std::cerr << "Helpers::mean: empty.";
        return Eigen::MatrixXd(0,0);
    }

    Eigen::MatrixXd accum = Eigen::MatrixXd::Constant(input.at(0).rows(),1, 0.0);
    for(size_t i = 0; i < n; i++)
        accum = accum + input.at(i);

    accum = accum / static_cast<double>(n);

    return accum;
}
