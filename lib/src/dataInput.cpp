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

#include "dataInput.h"

#include <set>
#include <iostream>

using namespace std;

DataInput::DataInput( )
{
}

DataInput::~DataInput()
{
}
void DataInput::addTrainingSample(const Eigen::MatrixXd &input, const Eigen::MatrixXd &expectedOutput)
{
    DataElement de;
    de.input = input;
    de.output = expectedOutput;
    de.outputSet = true;
    m_training.push_back(de);
}

void DataInput::addTrainingSample(const Eigen::MatrixXd &input, int lable)
{
    DataElement de;
    de.input = input;
    de.lable = lable;
    de.lableSet = true;
    m_training.push_back(de);
}


void DataInput::addTestSample(const Eigen::MatrixXd &input, const Eigen::MatrixXd &expectedOutput)
{
    DataElement de;
    de.input = input;
    de.output = expectedOutput;
    de.outputSet = true;
    m_test.push_back(de);
}

void DataInput::addTestSample(const Eigen::MatrixXd &input, int lable)
{
    DataElement de;
    de.input = input;
    de.lable = lable;
    de.lableSet = true;
    m_test.push_back(de);
}

void DataInput::clear()
{
    m_training.clear();
    m_test.clear();
    m_lables.clear();
}

size_t DataInput::getNumberOfTrainingSamples() const
{
    return m_training.size();
}

size_t DataInput::getNumberOfTestSamples() const
{
    return m_test.size();
}

bool DataInput::generateFromLables()
{
    std::set<int> lables;

    // count number lables in training
    for( DataElement& de : m_training )
    {
        if (de.lableSet)
        {
            int currentLable = de.lable;
            lables.insert(currentLable);
        }
        else
        {
            std::cerr << "Lable not set" << std::endl;
            return false;
        }
    }

    size_t nbrOfLables = lables.size();

    // generate output vector for each lable
    // the vector is of dimension nbrOfLables x 1
    // with all elements 0.0 but the lable position is 1.0.
    // Example for nbrOfLables = 3 and second lable -> [0,1,0]

    m_lables.clear();
    Eigen::MatrixXd outputBase = Eigen::MatrixXd::Constant(nbrOfLables,1, 0.0);

    size_t currentLableIdx = 0;
    for (int lNbr: lables)
    {
        Eigen::MatrixXd thisOut = outputBase;
        thisOut(currentLableIdx,0) = 1.0;

        m_lables.insert(std::pair<int,DataLable>(lNbr,DataLable(lNbr,thisOut)));

        currentLableIdx++;
    }

    // for each lable, m_lables has the corresponding output vector

    // lets assign the generated output vectors to the test and training samples
    if( !assignOutput(m_training) )
        return false;

    if( !assignOutput(m_test) )
        return false;

    return true;
}

bool DataInput::assignOutput( std::vector<DataElement>& vector )
{
    for( DataElement& de : vector )
    {
        if (de.lableSet)
        {
            auto foundLable = m_lables.find(de.lable);
            if( foundLable != m_lables.end() )
            {
                de.outputSet = true;
                de.output = (*foundLable).second.output;
            }
            else
            {
                std::cerr << "Unknown lable" << std::endl;
                return false;
            }
        }
        else
        {
            std::cerr << "Lable not set" << std::endl;
            return false;
        }
    }

    return true;
}

std::vector<Eigen::MatrixXd> DataInput::getInputData( const std::vector<DataElement>& vector )
{
    std::vector<Eigen::MatrixXd> ret;

    for( const DataElement& de : vector )
    {
        ret.push_back(de.input);
    }

    return ret;
}

std::vector<Eigen::MatrixXd> DataInput::getOutputData( const std::vector<DataElement>& vector )
{
    std::vector<Eigen::MatrixXd> ret;

    for( const DataElement& de : vector )
    {
        if( de.outputSet )
        {
            ret.push_back(de.output);
        }
        else
        {
            std::cout << "Warning, no output set." << std::endl;
        }
    }

    return ret;
}