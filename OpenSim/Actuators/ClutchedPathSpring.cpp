/* -------------------------------------------------------------------------- *
 *                      OpenSim:  ClutchedPathSpring.cpp                      *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2012 Stanford University and the Authors                *
 * Author(s): Ajay Seth                                                     *
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

//=============================================================================
// INCLUDES
//=============================================================================
#include "ClutchedPathSpring.h"


//=============================================================================
// STATICS
//=============================================================================
using namespace std;
using namespace OpenSim;
using SimTK::Vec3;

static const Vec3 DefaultClutchedPathSpringColor(.9,.9,.9); // mostly white 

//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
// Default constructor.
ClutchedPathSpring::ClutchedPathSpring()
{
	constructProperties();
}

ClutchedPathSpring::ClutchedPathSpring(const string& name, double stiffness,
					double dissipation, double relaxationTau, double stretch0)
{
	constructProperties();
	setName(name);
	set_stiffness(stiffness);
	set_dissipation(dissipation);
	set_relaxation_time_constant(relaxationTau);
	set_initial_stretch(stretch0);
}

//_____________________________________________________________________________
/*
 * Construct and initialize the properties for the ClutchedPathSpring.
 */
void ClutchedPathSpring::constructProperties()
{
	setAuthors("Ajay Seth");
	constructProperty_stiffness(SimTK::NaN);
	constructProperty_dissipation(SimTK::NaN);
	constructProperty_relaxation_time_constant(0.001); //1ms
	constructProperty_initial_stretch(0.0);
	setMinControl(0.0);
	setMaxControl(1.0);

	setOptimalForce(1.0);
}

//_____________________________________________________________________________
/*
 * Set the stiffness.
 */
void ClutchedPathSpring::setStiffness(double stiffness)
{
	set_stiffness(stiffness);
}
//_____________________________________________________________________________
/*
 * Set the dissipation.
 */
void ClutchedPathSpring::setDissipation(double dissipation)
{
	set_dissipation(dissipation);
}

//_____________________________________________________________________________
/*
 * Set the initial stretch.
 */
void ClutchedPathSpring::setInitialStretch(double stretch0)
{
	set_initial_stretch(stretch0);
}

//_____________________________________________________________________________
/**
 * allocate and initialize the SimTK state for this ClutchedPathSpring.
 */
 void ClutchedPathSpring::addToSystem(SimTK::MultibodySystem& system) const
{
	Super::addToSystem(system);
	// The spring force is dependent of stretch so only invalidate dynamics
	// if the stretch state changes
	addStateVariable("stretch");
}

 void ClutchedPathSpring::initStateFromProperties(SimTK::State& state) const
 {
	 setStateVariable(state, "stretch", get_initial_stretch());
 }

 void ClutchedPathSpring::setPropertiesFromState(const SimTK::State& state)
 {
	 set_initial_stretch(getStretch(state));
 }
 
 Array<std::string> ClutchedPathSpring::getStateVariableNames() const
 {
	 Array<std::string> stateVariableNames = ModelComponent::getStateVariableNames();
	 // Make state variable names unique to this muscle
	 for(int i=0; i<stateVariableNames.getSize(); ++i){
		 stateVariableNames[i] = getName()+"."+stateVariableNames[i];
	 }
	 return stateVariableNames;
 }

 SimTK::SystemYIndex ClutchedPathSpring::
	 getStateVariableSystemIndex(const string& stateVariableName) const
 {
	 unsigned start = (unsigned)stateVariableName.find(".");
	 unsigned end = (unsigned)stateVariableName.length();

	 if(start == end)
		 return ModelComponent::getStateVariableSystemIndex(stateVariableName);
	 else{
         ++start;
		 string localName = stateVariableName.substr(start, end-start);
		 return ModelComponent::getStateVariableSystemIndex(localName);
	 }
 }


//=============================================================================
// GET AND SET
//=============================================================================
//-----------------------------------------------------------------------------
// STRETCH and TENSION
//-----------------------------------------------------------------------------

double ClutchedPathSpring::getStretch(const SimTK::State& s) const
{
	return getStateVariable(s, "stretch");
}


double ClutchedPathSpring::getTension(const SimTK::State& s) const
{
	// evaluate tension in the spring
	// note tension is positive and produces shortening
	// damping opposes lengthening, which is positive lengthening speed
	// there for stretch and lengthening speed increase tension
	return getForce(s); 
}


//-----------------------------------------------------------------------------
// COMPUTATIONS
//-----------------------------------------------------------------------------
double ClutchedPathSpring::computeActuation(const SimTK::State& s) const
{
	// clamp or cap the control input to [0, 1]
	double control = SimTK::clamp(0.0, getControl(s), 1.0); 
	double tension = control *
				(getStiffness()*getStretch(s) *					//elastic force
				(1+getDissipation()*getLengtheningSpeed(s)));   //dissipation 
	
	setForce(s, tension);
	return tension;
}

SimTK::Vector ClutchedPathSpring::
	computeStateVariableDerivatives(const SimTK::State& s) const
{
	SimTK::Vector derivs(1, 0.0);

	double ldot = getLengtheningSpeed(s);

	derivs[0] = getControl(s) > SimTK::SignificantReal ? // non-zero control
					ldot : // clutch is engaged
					-getStretch(s)/get_relaxation_time_constant();

	return derivs;
} 

SimTK::Vec3 ClutchedPathSpring::computePathColor(const SimTK::State& state) const 
{
	double shade = SimTK::clamp(0.1, getControl(state), 1.0);
    const SimTK::Vec3 color(shade, 0.9, 0.1); // green to yellow
    return color;
}
