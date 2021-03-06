/* -------------------------------------------------------------------------- *
 *                 OpenSim:  ActivationFiberLengthMuscle.cpp                  *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2012 Stanford University and the Authors                *
 * Author(s): Ajay Seth                                                       *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

//==============================================================================
// INCLUDES
//==============================================================================
#include "ActivationFiberLengthMuscle.h"
#include "Model.h"

using namespace std;
using namespace OpenSim;
using SimTK::Vec3;


//==============================================================================
// STATIC INITIALIZATION
//==============================================================================
const string ActivationFiberLengthMuscle::STATE_ACTIVATION_NAME = "activation";
const string ActivationFiberLengthMuscle::STATE_FIBER_LENGTH_NAME = "fiber_length";


//==============================================================================
// CONSTRUCTORS
//==============================================================================
// Uses default (compiler-generated) destructor, copy constructor, and copy
// assignment operator.

//_____________________________________________________________________________
// Default constructor.
ActivationFiberLengthMuscle::ActivationFiberLengthMuscle()
{
	constructProperties();
}

//_____________________________________________________________________________
// Allocate and initialize properties.
void ActivationFiberLengthMuscle::constructProperties()
{
	constructProperty_default_activation(0.05);
	constructProperty_default_fiber_length(getOptimalFiberLength());
}

//_____________________________________________________________________________
// Allocate Simbody System resources for this actuator.
 void ActivationFiberLengthMuscle::addToSystem(SimTK::MultibodySystem& system) const
{
	Super::addToSystem(system);   // invoke superclass implementation

	const string& className = getConcreteClassName();
	const string& suffix = " flag is not currently implemented.";

	if(get_ignore_activation_dynamics()){
		string errMsg = className + "::ignore_activation_dynamics" + suffix;
		throw Exception(errMsg);
	}

	if(get_ignore_tendon_compliance()){
		string errMsg = className + "::ignore_tendon_compliance" + suffix;
		throw Exception(errMsg);
	}

	addStateVariable(STATE_ACTIVATION_NAME);
	// Fiber length should be a position stage state variable.
	// That is setting the fiber length should force position and above
	// dependent cache to be reevaluated. Problem with doing this now
	// is that there are no position level Z variables and making it 
	// a Q (multibody position coordinate) will invalidate the whole
	// multibody position realization which is overkill and would
	// also wipe out the muscle path, which we do not want to 
	// reevaluate over and over.
	addStateVariable(STATE_FIBER_LENGTH_NAME);//, SimTK::Stage::Velocity);

	double value = 0.0;
	addCacheVariable(STATE_ACTIVATION_NAME+"_deriv", value, SimTK::Stage::Dynamics);
	addCacheVariable(STATE_FIBER_LENGTH_NAME+"_deriv", value, SimTK::Stage::Dynamics);
 }

 void ActivationFiberLengthMuscle::initStateFromProperties( SimTK::State& s) const
{
    Super::initStateFromProperties(s);   // invoke superclass implementation

	ActivationFiberLengthMuscle* mutableThis = const_cast<ActivationFiberLengthMuscle *>(this);

	setActivation(s, getDefaultActivation());
	setFiberLength(s, getDefaultFiberLength());
}

void ActivationFiberLengthMuscle::setPropertiesFromState(const SimTK::State& state)
{
	Super::setPropertiesFromState(state);    // invoke superclass implementation

    setDefaultActivation(getStateVariable(state, STATE_ACTIVATION_NAME));
    setDefaultFiberLength(getStateVariable(state, STATE_FIBER_LENGTH_NAME));
}

void ActivationFiberLengthMuscle::connectToModel(Model& aModel)
{
    Super::connectToModel(aModel);
}

double ActivationFiberLengthMuscle::getDefaultActivation() const {
    return get_default_activation();
}
void ActivationFiberLengthMuscle::setDefaultActivation(double activation) {
    set_default_activation(activation);
}
double ActivationFiberLengthMuscle::getDefaultFiberLength() const {
    return get_default_fiber_length();
}
void ActivationFiberLengthMuscle::setDefaultFiberLength(double length) {
    set_default_fiber_length(length);
}

//_____________________________________________________________________________
/**
 * Get the name of a state variable, given its index.
 *
 * @param aIndex The index of the state variable to get.
 * @return The name of the state variable.
 */
Array<std::string> ActivationFiberLengthMuscle::getStateVariableNames() const
{
	Array<std::string> stateVariableNames = ModelComponent::getStateVariableNames();
	// Make state variable names unique to this muscle
	for(int i=0; i<stateVariableNames.getSize(); ++i){
		stateVariableNames[i] = getName()+"."+stateVariableNames[i];
	}
	return stateVariableNames;
}

// STATES
//_____________________________________________________________________________
/**
 * Set the derivative of an actuator state, specified by name
 *
 * @param aStateName The name of the state to set.
 * @param aValue The value to set the state to.
 */
void ActivationFiberLengthMuscle::
setStateVariableDeriv(const SimTK::State& s, const std::string &aStateName, 
                      double aValue) const
{
	double& cacheVariable = updCacheVariable<double>(s, aStateName + "_deriv");
	cacheVariable = aValue;
	markCacheVariableValid(s, aStateName + "_deriv");
}

//_____________________________________________________________________________
/**
 * Get the derivative of an actuator state, by index.
 *
 * @param aStateName the name of the state to get.
 * @return The value of the state.
 */
double ActivationFiberLengthMuscle::
getStateVariableDeriv(const SimTK::State& s, const std::string &aStateName) const
{
	return getCacheVariable<double>(s, aStateName + "_deriv");
}

//_____________________________________________________________________________
/**
 * Compute the derivatives of the muscle states.
 *
 * @param s  system state
 */
SimTK::Vector ActivationFiberLengthMuscle::
computeStateVariableDerivatives(const SimTK::State &s) const
{
	SimTK::Vector derivs(getNumStateVariables(), 0.);
    if (!isDisabled(s)) {
	    derivs[0] = getActivationRate(s);
	    derivs[1] = getFiberVelocity(s);
    }
	return derivs;
}


//==============================================================================
// GET
//==============================================================================
//-----------------------------------------------------------------------------
// STATE VALUES
//-----------------------------------------------------------------------------

void ActivationFiberLengthMuscle::setActivation(SimTK::State& s, double activation) const
{
	setStateVariable(s, STATE_ACTIVATION_NAME, activation);
}

void ActivationFiberLengthMuscle::setFiberLength(SimTK::State& s, double fiberLength) const
{
	setStateVariable(s, STATE_FIBER_LENGTH_NAME, fiberLength);
	// NOTE: This is a temporary measure since we were forced to allocate
	// fiber length as a Dynamics stage dependent state variable.
	// In order to force the recalculation of the length cache we have to 
	// invalidate the length info whenever fiber length is set.
	markCacheVariableInvalid(s,"lengthInfo");
    markCacheVariableInvalid(s,"velInfo");
    markCacheVariableInvalid(s,"dynamicsInfo");
    
}

double ActivationFiberLengthMuscle::getActivationRate(const SimTK::State& s) const
{
	return calcActivationRate(s);
}


//==============================================================================
// SCALING
//==============================================================================

//_____________________________________________________________________________
/**
 * Perform computations that need to happen after the muscle is scaled.
 * For this object, that entails updating the muscle path. Derived classes
 * should probably also scale or update some of the force-generating
 * properties.
 *
 * @param aScaleSet XYZ scale factors for the bodies.
 */
void ActivationFiberLengthMuscle::
postScale(const SimTK::State& s, const ScaleSet& aScaleSet)
{
	GeometryPath& path = upd_GeometryPath();

	path.postScale(s, aScaleSet);

	if (path.getPreScaleLength(s) > 0.0)
		{
			double scaleFactor = getLength(s) / path.getPreScaleLength(s);
			upd_optimal_fiber_length() *= scaleFactor;
			upd_tendon_slack_length() *= scaleFactor;
			path.setPreScaleLength(s, 0.0) ;
		}
}

//--------------------------------------------------------------------------
// COMPUTATIONS
//--------------------------------------------------------------------------
//The concrete classes are the only ones qualified to write this routine.
/*void ActivationFiberLengthMuscle::computeInitialFiberEquilibrium(SimTK::State& s) const
{
	// Reasonable initial activation value
	//cout << getName() << "'s activation is: " << getActivation(s) << endl;
	//cout << getName() << "'s fiber-length is: " << getFiberLength(s) << endl;
	if(getActivation(s) < 0.01){
		setActivation(s, 0.01);
	}
	//cout << getName() << "'s NEW activation is: " << getActivation(s) << endl;
	setFiberLength(s, getOptimalFiberLength());
	_model->getMultibodySystem().realize(s, SimTK::Stage::Velocity);
	double force = computeIsometricForce(s, getActivation(s));
	//cout << getName() << "'s Equilibrium activation is: " << getActivation(s) << endl;
	//cout << getName() << "'s Equilibrium fiber-length is: " << getFiberLength(s) << endl;
}*/

//==============================================================================
// FORCE APPLICATION
//==============================================================================
//_____________________________________________________________________________
/**
 * Apply the muscle's force at its points of attachment to the bodies.
 */
void ActivationFiberLengthMuscle::computeForce(const SimTK::State& s, 
							  SimTK::Vector_<SimTK::SpatialVec>& bodyForces, 
							  SimTK::Vector& generalizedForces) const
{
	Muscle::computeForce(s, bodyForces, generalizedForces);

	if( isForceOverriden(s) ) {
		// Also define the state derivatives, since realize acceleration will
		// ask for muscle derivatives, which will be integrated
		// in the case the force is being overridden, the states aren't being used
		// but a valid derivative cache entry is still required
		int numStateVariables = getNumStateVariables();
		Array<std::string> stateVariableNames = getStateVariableNames();
		for (int i = 0; i < numStateVariables; ++i) {
			stateVariableNames[i] = stateVariableNames[i].substr(stateVariableNames[i].find('.') + 1);
			setStateVariableDeriv(s, stateVariableNames[i], 0.0);
		}
    } 

}




SimTK::SystemYIndex ActivationFiberLengthMuscle::
getStateVariableSystemIndex(const string &stateVariableName) const
{
	unsigned int start = (int)stateVariableName.find(".");
	unsigned int end = (int)stateVariableName.length();
	
	if(start == end)
		return ModelComponent::getStateVariableSystemIndex(stateVariableName);
	else{
        ++start;
		string localName = stateVariableName.substr(start, end-start);
		return ModelComponent::getStateVariableSystemIndex(localName);
	}
}