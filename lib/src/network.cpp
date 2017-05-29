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

#include "network.h"
#include "layer.h"

#include <random>
#include <iostream>

using namespace std;

Network::Network( const vector<unsigned int> networkStructure ) :
    m_NetworkStructure( networkStructure )
{
    initNetwork();
}

Network::~Network()
{

}

void Network::initNetwork()
{
    unsigned int nbrOfInputs = 0; // for input layer, there is no input needed.

    for( unsigned int nbrOfNeuronsInLayer : m_NetworkStructure )
    {
        m_Layers.push_back( shared_ptr<Layer>( new Layer(nbrOfNeuronsInLayer, nbrOfInputs) ) );
        nbrOfInputs = nbrOfNeuronsInLayer; // the next layer has same number of inputs as neurons in this layer.
    }

    m_activation_out = Eigen::MatrixXd( 1, 1 ); // dimension will be updated based on nbr of input samples
}

bool Network::feedForward( const Eigen::MatrixXd& x_in )
{
    // first layer does not perform any operation. It's activation output is just x_in.
    if( ! getLayer(0)->setActivationOutput(x_in) )
        return false;

    for( unsigned int k = 1; k < m_Layers.size(); k++ )
    {
        // Pass output signal from former layer to next layer.
        if( ! getLayer( k )->feedForward(  getLayer( k-1 )->getOutputActivation()  ) )
        {
            cout << "Error: Outpt-Input signal size mismatch" << endl;
            return false;
        }
    }

    // network output signal is in the last layer
    m_activation_out = m_Layers.back()->getOutputActivation();

    return true;
}

unsigned int Network::getNumberOfLayer() const
{
    return unsigned(m_NetworkStructure.size());
}

shared_ptr<Layer> Network::getLayer( const unsigned int& layerIdx )
{
    if( layerIdx >= getNumberOfLayer() )
    {
        cout << "Error: getLayer out of index" << endl;
        return shared_ptr<Layer>(NULL);
    }

    return m_Layers.at(layerIdx);
}

bool Network::gradientDescent( const Eigen::VectorXd& x_in, const Eigen::VectorXd& y_out, const double& eta )
{
    if( ! doFeedforwardAndBackpropagation(x_in, y_out ) )
        return false;

    // Update weights and biases with the computed derivatives and learning rate.
    // First layer does not need to be updated -> it is just input layer
    for( unsigned int k = 1; k < getNumberOfLayer(); k++ )
        getLayer(k)->updateWeightsAndBiases( eta );

    return true;
}

bool Network::stochasticGradientDescent(const std::vector<Eigen::VectorXd>& samples, const std::vector<Eigen::VectorXd>& lables,
                                        const unsigned int& batchsize, const double& eta)
{
    if( samples.size() != lables.size() )
    {
        cout << "Error: number of samples and lables mismatch" << endl;
        return false;
    }

    size_t nbrOfSamples = samples.size();

    if( nbrOfSamples < batchsize )
    {
        cout << "Error: batchsize exceeds number of available smaples" << endl;
        return false;
    }

    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_int_distribution<> iDist(0, int(nbrOfSamples)-1);

    // initialize pd sum vectors and matrices - skip first layer (input layer)
    std::vector<Eigen::VectorXd> biasPDSum;
    std::vector<Eigen::MatrixXd> weightPDSum;
    for( unsigned int i = 1; i < getNumberOfLayer(); i++ )
    {
        const std::shared_ptr<Layer>& l = getLayer(i);
        Eigen::VectorXd biasPDInit = Eigen::VectorXd::Constant( l->getNbrOfNeurons(), 0.0 );
        Eigen::MatrixXd weightPDInit = Eigen::MatrixXd::Constant( l->getNbrOfNeurons(), l->getNbrOfNeuronInputs(), 0.0 );
        biasPDSum.push_back( biasPDInit );
        weightPDSum.push_back( weightPDInit );
    }


    for( unsigned int b = 0; b < batchsize; b++ )
    {
        // randomly choose a sample
        size_t rIdx = size_t( iDist(e2) );
        doFeedforwardAndBackpropagation( samples.at(rIdx), lables.at(rIdx) );

        // accumulate partial derivatives of each layer
        for( unsigned int j = 1; j < getNumberOfLayer(); j++ )
        {
            const std::shared_ptr<Layer>& l = getLayer(j);
            biasPDSum.at(j-1) = biasPDSum.at(j-1) + l->getPartialDerivativesBiases();
            weightPDSum.at(j-1) = weightPDSum.at(j-1) + l->getPartialDerivativesWeights();
        }
    }

    // update weights and biases with the average partial derivatives
    for( unsigned int j = 1; j < getNumberOfLayer(); j++ )
    {
        const std::shared_ptr<Layer>& l = getLayer(j);
        Eigen::VectorXd avgPDBias = biasPDSum.at(j-1) * ( eta / double(batchsize) );
        Eigen::MatrixXd avgPDWeights = weightPDSum.at(j-1) * ( eta / double(batchsize) );
        l->updateWeightsAndBiases(avgPDBias, avgPDWeights);
    }

    return true;
}

shared_ptr<Layer> Network::getOutputLayer()
{
    return getLayer( getNumberOfLayer() - 1 );
}

double Network::getNetworkErrorMagnitude()
{
    Eigen::VectorXd oErr =  getOutputLayer()->getBackpropagationError();
    return oErr.norm();
}

void Network::print()
{
    // skip first layer -> input
    for( unsigned int i = 1; i < getNumberOfLayer(); i++ )
    {
        shared_ptr<Layer> l = getLayer( i );

        std::cout << "Layer " << i << ":" << std::endl;
        getLayer( i )->print();
        std::cout << std::endl << std::endl;
    }
}

bool Network::doFeedforwardAndBackpropagation( const Eigen::VectorXd& x_in, const Eigen::VectorXd& y_out )
{
    // updates output in all layers
    if( ! feedForward(x_in) )
        return false;

    if( getOutputActivation().rows() != y_out.rows() )
    {
        cout << "Error: desired output signal mismatching dimension" << endl;
        return false;
    }

    // Compute output error in the last layer
    std::shared_ptr<Layer> layerAfter = getOutputLayer();
    layerAfter->computeBackpropagationOutputLayerError( y_out );
    layerAfter->computePartialDerivatives();

    // Compute error and partial derivatives in all remaining layers, but not input layer
    for( int k = int(getNumberOfLayer()) - 2; k > 0; k-- )
    {
        std::shared_ptr<Layer> thisLayer = getLayer( unsigned(k) );
        thisLayer->computeBackprogationError( layerAfter->getBackpropagationError(), layerAfter->getWeigtMatrix() );
        thisLayer->computePartialDerivatives();

        layerAfter = thisLayer;
    }

    return true;
}
