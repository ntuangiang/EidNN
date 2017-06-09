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

#include <iostream>
#include "layer.h"
#include "neuron.h"
#include "helpers.h"

using namespace std;

Layer::Layer(const uint& nbr_of_neurons , const uint &nbr_of_inputs) :
    m_nbr_of_neurons( nbr_of_neurons ),
    m_nbr_of_inputs( nbr_of_inputs )
{
    initLayer();
}

Layer::Layer( const uint& nbr_of_inputs, const vector<Eigen::VectorXd>& weights, const vector<double>& biases ) :
    Layer::Layer( uint(weights.size()), nbr_of_inputs )
{
    assert( weights.size() ==  biases.size() );

    // write weight matrix and bias vector
    for( unsigned int n = 0; n < weights.size(); n++ )
    {
        m_weightMatrix.row(n) = weights.at(n).transpose();
        m_biasVector(n,0) = biases.at(n);
    }
}

Layer::Layer( const Layer& l ) : Layer( l.getNbrOfNeurons(), l.getNbrOfNeuronInputs() )
{
    // Note: Temporary results like activations and derivatives are not copied.
    m_weightMatrix = l.getWeigtMatrix();
    m_biasVector = l.getBiasVector();
}

// init vectors and neurons
void Layer::initLayer()
{
    m_weightMatrix = Eigen::MatrixXd( m_nbr_of_neurons , m_nbr_of_inputs );
    m_biasVector = Eigen::MatrixXd( m_nbr_of_neurons, 1 );
    resetRandomlyWeightsAndBiases();

    // init with size 1 -> dimensionso of these matrices will change corrsponding to input signal
    m_activation_in = Eigen::MatrixXd( 1, 1 );
    m_activation_out = Eigen::MatrixXd( 1, 1 );
    m_z_weighted_input = Eigen::MatrixXd( 1, 1 );
    m_backpropagationError =  Eigen::MatrixXd( 1 , 1 );
}


Layer::~Layer()
{
    // used smart pointers
}

bool Layer::feedForward(const Eigen::MatrixXd &x_in )
{
    if( x_in.rows() != m_nbr_of_inputs )
    {
        std::cout << "Error: Layer input vector size mismatch" << std::endl;
        return false;
    }

    m_activation_in = x_in;
    m_z_weighted_input = m_weightMatrix * x_in + m_biasVector.replicate(1, x_in.cols());

    // compute sigmoid for weighted input vector
    m_activation_out = Eigen::MatrixXd(m_z_weighted_input.rows(), m_z_weighted_input.cols());
    for( unsigned int m = 0; m < m_z_weighted_input.rows(); m++ )
        for( unsigned int n = 0; n < m_z_weighted_input.cols(); n++ )
            m_activation_out(m,n) = Neuron::sigmoid( m_z_weighted_input(m,n) );

    return true;
}


bool Layer::setWeights( const vector<Eigen::VectorXd>& weights )
{
    if( weights.size() != getNbrOfNeurons() )
    {
        std::cout << "Error: Weights vector size mismatches number of neurons" << std::endl;
        return false;
    }

    for( unsigned int n = 0; n < weights.size(); n++ )
        m_weightMatrix.row(n) = weights.at(n).transpose();

    return true;
}

bool Layer::setWeights( const Eigen::MatrixXd& weights )
{
    if( weights.rows() != m_weightMatrix.rows() || weights.cols() != m_weightMatrix.cols() )
    {
        std::cout << "Error: Weights matrix size mismatches" << std::endl;
        return false;
    }

    m_weightMatrix = weights;
    return true;
}

bool Layer::setBiases( const vector<double>& biases )
{
    if( biases.size() != getNbrOfNeurons() )
    {
        std::cout << "Error: Bias vector size mismatches number of neurons" << std::endl;
        return false;
    }

    for( unsigned int n = 0; n < biases.size(); n++ )
        m_biasVector(n,0) = biases.at(n);

    return true;
}

bool Layer::setBiases( const Eigen::MatrixXd& biases )
{
    if( biases.rows() != getNbrOfNeurons() || biases.cols() != 1 )
    {
        std::cout << "Error: Bias Eigen vector size mismatches" << std::endl;
        return false;
    }

    m_biasVector = biases;

    return true;
}

void Layer::setWeight( const double& weight )
{
    Eigen::VectorXd uniformWeight = Eigen::VectorXd::Constant(getNbrOfNeuronInputs(), weight);

    for( unsigned int n = 0; n < getNbrOfNeurons(); n++ )
        m_weightMatrix.row(n) = uniformWeight.transpose();
}


void Layer::setBias(const double &bias )
{
    m_biasVector = Eigen::MatrixXd::Constant(getNbrOfNeurons(), 1, bias);
}


void Layer::resetRandomlyWeightsAndBiases()
{
    std::default_random_engine randomGenerator;
    std::normal_distribution<double> gNoise(0.0, 1.0);

    for( unsigned int i = 0; i < getNbrOfNeurons(); i++ )
    {
        m_biasVector(i,0) = gNoise(randomGenerator);

        Eigen::VectorXd thisWeights = Eigen::VectorXd( getNbrOfNeuronInputs() );
        for( unsigned int k = 0; k < getNbrOfNeuronInputs(); k++ )
            thisWeights(k) = gNoise(randomGenerator);

        m_weightMatrix.row(i) = thisWeights.transpose();
    }
}


bool Layer::setActivationOutput( const Eigen::MatrixXd& activation_out )
{
    if( activation_out.rows() != getNbrOfNeurons() )
    {
        std::cout << "Error: Activation output signal mismatch" << std::endl;
        return false;
    }

    m_activation_out = activation_out;
    return true;
}

bool Layer::computeBackpropagationOutputLayerError(const Eigen::MatrixXd &expectedNetworkOutput )
{
    if( m_activation_out.rows() != expectedNetworkOutput.rows() ||
            m_activation_out.cols() != expectedNetworkOutput.cols())
    {
        std::cout << "Error: Layer activation output to label mismatch" << std::endl;
        return false;
    }

    m_backpropagationError = ((m_activation_out - expectedNetworkOutput).array() * d_sigmoid( m_z_weighted_input ).array()).matrix();
    return true;
}

bool Layer::computeBackprogationError(const Eigen::MatrixXd &errorNextLayer, const Eigen::MatrixXd& weightMatrixNextLayer )
{
    if( m_z_weighted_input.rows() != weightMatrixNextLayer.cols()  ||  errorNextLayer.rows() != weightMatrixNextLayer.rows() )
    {
        std::cout << "Error: computeBackprogationError Layer dimension mismatch" << std::endl;
        return false;
    }

    m_backpropagationError = ((weightMatrixNextLayer.transpose() * errorNextLayer).array() * d_sigmoid( m_z_weighted_input ).array()).matrix();
    return true;
}


const Eigen::MatrixXd Layer::d_sigmoid( const Eigen::MatrixXd& z )
{
    Eigen::MatrixXd res = Eigen::MatrixXd( z.rows(), z.cols() );
    for( unsigned int m = 0; m < z.rows(); m++ )
        for( unsigned int n = 0; n < z.cols(); n++ )
            res(m,n) = Neuron::d_sigmoid( z(m,n) );

    return res;
}

void Layer::computePartialDerivatives()
{
    Eigen::MatrixXd delta = getBackpropagationError();

    m_bias_partialDerivatives.clear();
    m_weight_partialDerivatives.clear();

    // compute derivatives for each passed sample
    for( unsigned int k = 0; k < delta.cols(); k++ )
    {
        Eigen::MatrixXd thisDelta = delta.col(k);
        m_bias_partialDerivatives.push_back( thisDelta );

        Eigen::MatrixXd thisInputActivation = getInputActivation().col(k);
        m_weight_partialDerivatives.push_back( delta.col(k) * thisInputActivation.transpose() ); // This is different from the 4th-equation? Study!
    }
}

void Layer::updateWeightsAndBiases(const double &eta, const unsigned int& sampleIdx )
{
    updateWeightsAndBiases(eta * getPartialDerivativesBiases().at(sampleIdx), eta * getPartialDerivativesWeights().at(sampleIdx) );
}

void Layer::updateWeightsAndBiases(const Eigen::MatrixXd& deltaBias, const Eigen::MatrixXd& deltaWeight)
{

    const Eigen::MatrixXd newBiases = getBiasVector() - deltaBias;
    setBiases( newBiases );

    const Eigen::MatrixXd newWeights = getWeigtMatrix() - deltaWeight;
    setWeights( newWeights );
}

void Layer::print() const
{
    Eigen::VectorXd a;

    Helpers::printVector(getBiasVector(),"Biases");
    Helpers::printMatrix(getWeigtMatrix(),"Weights");
    Helpers::printVector(getBackpropagationError(),"Error");
}




