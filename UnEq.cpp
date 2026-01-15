//#include "pch.h"
#include "UnEq.h"
#include "Var.h"
#include "VarGroup.h"
#include "OrchestraReader.h"
#include "ParameterList.h"
#include "IO.h"
#include "OrchestraException.h"
#include "stringhelper.h"

namespace orchestracpp
{

		UnEq *UnEq::createUnEq2(const std::string &line, VarGroup *vars)// throw(ReadException, IOException)
		{
			UnEq *uneq = new UnEq();
			uneq->initialize(line, vars);
			uneq->isType3 = false;
			return uneq;
		}

		UnEq *UnEq::createUnEq3(const std::string &line, VarGroup *vars)// throw(ReadException, IOException)
		{
			UnEq *uneq = new UnEq();
			uneq->initialize(line, vars);
			uneq->isType3 = true;
			return uneq;
		}

		void UnEq::initialize(const std::string &line, VarGroup *vars) //throw(ReadException, IOException)
		{

			OrchestraReader *in_stream = new OrchestraReader(line);
			std::string word = StringHelper::toLower(in_stream->readWord());

			do
			{
				if (word == "unknown:")
				{
					ParameterList parameters(in_stream);

					for (int n = 0; n < parameters.size() - 1; n += 2)
					{
						std::string name = StringHelper::toLower(parameters.get(n));
						std::string value = parameters.get(n + 1);
							if (name == "name:") {
								unknown = vars->get(value);
								vars->addToGlobalVariables(unknown);
								unknown->isUnknown = true;
							}
							else if(name == "type:"){
								if (value == "lin") {
									un_type = lin;
								}
								else if (value == "log") {
									un_type = log;
									un_max_abs_step = 1;
								}else{
									IO::println("log or lin expected in uneq2 definition");
								}
							}
							else if (name == "min:") {
								un_min = std::stod(value);
							}
							else if (name == "default:") {
								unknown->setValue(std::stod(value));
							}
							else if (name == "max:") {
								un_max = std::stod(value);
							}
							else if (name == "delta:") {
								un_min_delta = std::stod(value);
							}
							else if (name == "reldelta:") {
								un_rel_delta = std::stod(value);
							}
							else if (name == "step:") {
								un_max_abs_step = std::stod(value);
							}
							else if ((name == "initiallyinactive:") || (name == "iia:")) {
								if (value == "true")
								{
									initiallyInactive = true;
									this->active = false;
								}
							}
							else {
								IO::println("Warning: " + name + " is not a valid keyword in a uneq definition");
							}
						
					}
				}
				else if (word == "equation:")
				{
					ParameterList parameters(in_stream);
					for (int n = 0; n < parameters.size() - 1; n += 2)
					{
						std::string name = StringHelper::toLower(parameters.get(n));
						std::string value = parameters.get(n + 1);
						if (name == "name:") {
							equation = vars->get(value);
							if (equation == nullptr)
							{
								throw ReadException(value + " was not defined as a variable!");
							}
							nanMessage = "NAN: " + this->equation->name;
						}
						else if (name == "si:") {
							siVariable = vars->get(value);
							if (siVariable == nullptr)
							{
								throw ReadException(value + " was not defined as a variable!");
							}

							siVariable->setConstant(false);

						}
						else if (name == "type:") {
								//                            if (value.equals("lin")) {
								//                                eq_type = lin;
								//                            }   if (value.equals("log")) {
								//                                eq_type = log;
								//                            }   
							
						}
						else if (name == "default:"){
							equation->setValue(std::stod(value));
						}
						else if (name == "tol:") {
							// tolerance can be a variable or a number
							toleranceVariable = vars->get(value);
							if (toleranceVariable == nullptr)
							{ // not a variable
								eq_tolerance = std::stod(value);
							}
							else
							{ // a variable
								eq_tolerance = toleranceVariable->getValue();
								toleranceVariable->usedForIO = true;
							}

						}
						else{
								IO::println("Warning: " + name + " is not a valid keyword in a uneq definition");
						}
					}
				}
				word = StringHelper::toLower(in_stream->readWord());
			} while (!in_stream->ready);

			unknown->setConstant(false); // the unknown variable is not a constant
			unknown->usedForIO = true;

			equation->setConstant(false); // the equation variable is not a constant
			equation->usedForIO = true;
			equation->isEquation = true;

			vars->addToGlobalVariables(unknown);

		}








        // this one was used until March2020
		void UnEq::dddetermineDeltaUnknown()
		{
			// so we use relative delta until close to zero
			// then we use min delta

			un_delta = unknown->getIniValue() * un_rel_delta;
			if (un_delta >= 0)
			{
				if (un_delta < un_min_delta)
				{
					un_delta = un_min_delta;
				}
			}
			else
			{ // un_delta < 0
				if (un_delta > -un_min_delta)
				{
					un_delta = -un_min_delta;
				}
			}
		}

		// this one was used after March2020 needs to be tested!!

		void UnEq::determineDeltaUnknown() { // this one does not work yet


			if (un_type == lin) {
				if (unknown->getIniValue() >= 0) {
					un_delta = 1e-3;
				}
				else {
					// inivalue<0
					un_delta = -1e-3;
					//IO.println(unknown.name+""+un_delta);
				}
			}
			else {// not linear so use relative delta
				un_delta = unknown->getIniValue() * un_rel_delta;
				if (un_delta >= 0) {
					if (un_delta < un_min_delta) {
						un_delta = un_min_delta;
					}
				}
				else {// un_delta < 0
					if (un_delta > -un_min_delta) {
						un_delta = -un_min_delta;
					}
				}
			}

		}


		double UnEq::checkUnknownStep()
		{
			double step;
			step = -centralResidual;

			if (step == 0.0)
			{
				factor = 1;
				return factor;
			}


			double value = unknown->getIniValue();

			if ((step > 0) && (step > un_max_abs_step))
			{
				step = un_max_abs_step;
			}
			else if ((step < 0) && (step < -un_max_abs_step))
			{
				step = -un_max_abs_step;
			}

			if ((value + step) > un_max)
			{
				step = (un_max + value) / 2 - value;
			}
			else if ((value + step) < un_min)
			{
				step = (un_min + value) / 2 - value;
			}

			factor = (step / -centralResidual);

			return factor;
		}

		void UnEq::updateUnknown(double factor)
		{
			//unknown.setValue(unknown.getIniValue() + -centralResidual * factor);

			if (un_type == lin)
			{
				unknown->setValue(unknown->getIniValue() + -centralResidual * factor);
			}
			else
			{ //log
				unknown->setValue(unknown->getIniValue() * std::pow(10, -centralResidual * factor));
			}



		}

		/*
         * <1 is convergent, indicates that residual < tolerance
         * larger than 1 means less convergent
         *
         */

		double UnEq::howConvergent() //throw(OrchestraException)
		{
			if (toleranceVariable != nullptr)
			{
				return std::abs(IO::checkNAN(residual(), nanMessage)) / toleranceVariable->getValue();
			}
			else
			{
				return std::abs(IO::checkNAN(residual(), nanMessage)) / eq_tolerance;
			}
		}

		bool UnEq::isConvergent()// throw(OrchestraException)
		{
			return (howConvergent() < 1.0);
		}

		double UnEq::offsetUnknown()
		{
			determineDeltaUnknown();
			double tmp = unknown->getIniValue();

			if (un_type == lin)
			{
				unknown->setValue(tmp + un_delta);
			}
			else
			{ // un_type == log
				unknown->setValue(tmp * std::pow(10, un_delta));
			}
			return tmp;
		}

		void UnEq::resetUnknown(double tmp)
		{
			unknown->setValue(tmp);
		}

		double UnEq::residual() //throw(OrchestraException)
		{
			// checking for NAN takes a lot of time!
			//return IO.checkNAN((equation.getValue() - equation.getIniValue()), "Encountered a NAN for: "+equation.name+" value: "+equation.getValue()+" ini value: "+equation.getIniValue());
			return (equation->getValue() - equation->getIniValue());
		}

		void UnEq::calculateCentralResidual() //throw(OrchestraException)
		{
			centralResidual = residual();
		}

		void UnEq::calculateJResidual() //throw(OrchestraException)
		{
			jacobianResidual = residual();
		}
	}
