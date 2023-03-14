//--------------------------------------------------------------------------------------------------------------------------
// This file is part of the C++ ORCHESTRA chemical solver code
//
// Copyright 2023, J.C.L. (Hans) Meeussen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//--------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// ORCHESTRA chemical solver module, C++ version
// Version 2023
// 
// This set of C++ files represents the complete ORCHESTRA (chemical) solver module.
// It is a literal translation of the the original Java version and was prepared to facilitate coupling to other (e.g. mass transport) codes,
// and to make it easier to call the solver directly from Python.
// The C++ version has (should have) exactly the same functionality as the Java version and can read chemical system definitions from the same text input files.
// Chemical input files can be created interactively with the Java GUI.
//
// Because ORCHESTRA reads all chemical models (variables / equations etc) from input text files at run time, 
// the C++ version automatically has the same chemical modelling capabilities as the Java version.
// (No chemical model definitions are present in this source code.)
// 
// An example of how the ORCHESTRA solver can be used is present in the "testOrchestra2.cpp" file.
// 
// Translation of the ORCHESTRA solver from Java to C++  was funded by the EURAD DONUT project.
// DONUT: Development and improvement of numerical methods and tools for modelling coupled processes
// 
// https://igdtp.eu/activity/donut-development-and-improvement-of-numerical-methods-and-tools-for-modelling-coupled-processes
//
// Hans Meeussen
// Nuclear Research and Consultancy Group (NRG): 	meeussen@nrg.eu, 
// Delft University of Technology: 			j.c.l.meeussen@tudelft.nl
// March 2023
// www.meeussen.nl/orchestra 
//---------------------------------------------------------------------------------------------------------------------------
