#pragma once

#include <string>
#include <cmath>
#include <limits>

#include "stringhelper.h"
#include "Var.h"
#include "UnEq.h"
#include "Var.h"
#include "VarGroup.h"
#include "OrchestraReader.h"
#include "ParameterList.h"
#include "IO.h"
#include "OrchestraException.h"
#include "stringhelper.h"

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:

namespace orchestracpp
{

		/**
		 * The Uneq (unknown equation) and UnEqGroup manage the iteration process within
		 * a calculator. a UnEq holds references to the unknown and equation variable. a
		 * UnEqsGroup contains a set of UnEqs. public methods of a_uneqs_group:
		 * a_uneqs_group(a_variables_group); constructor requires a_variables_group
		 * read_one(orchestrareader); adds a uneq to the group intitialize(); called
		 * after all uneqs are added. iterate(Calculator); manages the actual iteration,
		 * calls back the calculators calculate method
		 *
		 * @author Hans Meeussen 26/7/1999
		 * @changes jan 2004 addition of UnEq2 type
		 * 
		 * March 2018 removed UnEq class and renamed uneq2 to uneq
		 */
		class UnEq final
		{

		public:
			Var *unknown = nullptr;
			Var *equation = nullptr;

			double jacobianResidual = 0; // jacobian residual, equation value during jacobian calculation
			double centralResidual = 0; // central  residual value at given unknown values

			/* properties of the unknown variable */
			double un_min = -std::numeric_limits<double>::infinity(); // minimum value
			double un_max =  std::numeric_limits<double>::infinity(); // maximum value
			double un_rel_delta = 1e-5; // used relative delta
			double un_min_delta = 1e-70; // minimum absolute delta
			double un_max_abs_step = .1; // used absolute step when lazy
			double un_delta = 0; // delta unknown actually used

			double eq_tolerance = 1e-13; // what was the orignal default value?


			bool active = true; // indicates if this uneq is switched on during the iteration
			bool initiallyInactive = false;

			double factor = 0;

			//*****
			const int lin = 0;
			const int log = 1;
			int un_type = lin;
			//*****

			Var *siVariable = nullptr;
			Var *toleranceVariable = nullptr;

			bool isType3 = false;

			std::string nanMessage;

			virtual ~UnEq()
			{
				delete unknown;
				delete equation;
				delete siVariable;
				delete toleranceVariable;
			}

			static UnEq *createUnEq2(const std::string &line, VarGroup *vars)/* throw(ReadException, IOException)*/;

			static UnEq *createUnEq3(const std::string &line, VarGroup *vars)/* throw(ReadException, IOException)*/;

			virtual void initialize(const std::string &line, VarGroup *vars) /*throw(ReadException, IOException)*/;

		 /**
		     * calculate delta (change in unknown) unknown that is used in numerical
		     * determination of derivatives
		     */
			void determineDeltaUnknown();

			void dddetermineDeltaUnknown();



		   /**
		     * Determine if the proposed change in unknowns (-cresidual) is within the
		     * allowed domain and step size. 
		     * If so, the change in unknowns becomes -cresidual, 
		     * factor becomes 1. If not determine the allowed step, and
		     * determine the factor.
		     * 
		     * What happens if -cresidual is extremely small?
		     * can factor be larger than 1?
		     * can factor be negative?
		     * if residual is not a number?
		     */
			double checkUnknownStep();

			void updateUnknown(double factor);

			/*
			 * <1 is convergent
			 * larger than 1 means less convergent
			 */
		//    double howConvergent() throws OrchestraException{
		//        if (toleranceVariable!=null){
		//           return  (IO.checkNAN(Math.abs(residual()), "NAN: "+this.equation.name) / toleranceVariable.getValue());
		//        }else{
		//            return (IO.checkNAN(Math.abs(residual()), "NAN: "+this.equation.name)  / eq_tolerance);
		//        }
		//    }

			double howConvergent()/* throw(OrchestraException)*/;


			bool isConvergent() /*throw(OrchestraException)*/;

			double offsetUnknown();

			void resetUnknown(double tmp);


			/**
			 * This method returns the function value or residual that has to become
			 * zero
			 */
			double residual()/* throw(OrchestraException)*/;

			void calculateCentralResidual() /*throw(OrchestraException)*/;

			void calculateJResidual() /*throw(OrchestraException)*/;
		};

}
