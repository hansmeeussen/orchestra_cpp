#pragma once

#include <string>
#include <vector>
#include <cmath>
#include "VarGroup.h"
#include "Var.h"
#include "OrchestraReader.h"
#include "UnEq.h"
#include "StopFlag.h"
#include "OrchestraException.h"
//#include "

namespace orchestracpp
	{

		/**
		 * This UnEqGroup contains the uneqs (unknown-equations) and manages the
		 * iteration process
		 */
		class UnEqGroup
		{

		public:
			std::vector<UnEq*> uneqs;// = std::vector();
			double nrIter = 0; // nr of iterations at lowest level
			double totalNrIter = 0; // total number of iterations, including mineral iterations, new tries etc.
			double maxIter = 300;
			std::vector<UnEq*> activeUneqs;
			std::vector<std::vector<double>> jacobian;
			int nrActiveUneqs = 0;
			VarGroup *variables = nullptr;

			bool monitor = false; //switch on iteration monitoring
			FileWriter *iterationReport = nullptr;
			double originalMaxIter = 0;
			Var *minTol = nullptr;
			double minTolOrgValue = 0;
			Var *tolerance = nullptr;

			bool firstIteration = false; // if we set this at false, no iteration report is written 

			virtual ~UnEqGroup()
			{
				if (iterationReport != nullptr) {
					delete iterationReport;
				}
			}

			UnEqGroup(VarGroup *variables);

			/**
			 * This method is called AFTER all uneqs are added. and BEFORE each level1
			 * iteration. It dimensions the jacobian arrays according to the number of
			 * active uneqs.
			 */
			virtual void initialise();

			virtual UnEq *doesExist(UnEq *u) /*throw(ReadException)*/;


			virtual void read_one2(const std::string &infile) /*throw(ReadException, IOException)*/;

			virtual void read_one3(const std::string &infile) /*throw(ReadException, IOException)*/;

			/**
			 * This is the top level iteration method that is called from the calculator
			 * and manages the iteration process the calculate method of the calculator
			 * is used as a call-back method to perform a single calculation in the
			 * iteration process.
			 *
			 *
			 */
			virtual bool iterate(StopFlag *flag);

			virtual double getTotalNrIter();

			virtual double getNrIter();

			virtual bool getIIApresent();

			// switch on/off initially inactive uneqs
			virtual void switchOnIIA();

			virtual void switchOffIIA();

			int maxMineralIterations = 100; // shall we increase this?


			/**
			 * Here we iterate for the presence of minerals
			 * 
			 */
		private:
			void iterateLevelMinerals(StopFlag *flag) /*throw(IOException)*/;

		public:
			double howConvergent_field = 0;

		private:
			int iterateLevel0(Writer *iterationReportWriter, StopFlag *flag);

			/**
			 * This method is called when the iteration has failed, and was restarted to
			 * generate an iteration report. This method writes the header of this
			 * report.
			 *
			 * @param out
			 * @throws IOException
			 */
			void initialiseIterationReport() /*throw(IOException)*/;
			void initialiseIterationReport2() /*throw(IOException)*/;

			/**
			 * This methods writes a line of information to the iteration report
			 *
			 * @param out
			 * @param nrIter
			 * @throws IOException
			 * @throws OrchestraException
			 */
			void writeIterationReportLine(double nrIter) /*throw(IOException, OrchestraException)*/;
			void writeIterationReportLine2(double nrIter) /*throw(IOException, OrchestraException)*/;

			/**
			 * This method calculates the Jacobian matrix with partial derivatives. Note
			 * that the central function values (cresiduals) have already been
			 * calculated. Here we offset each unknown with its delta value and
			 * calculate the jacobian = delta equation / delta unknown
			 */
		public:
			virtual void calculateJacobian() /*throw(OrchestraException)*/;

			double commonfactor = 0;

			/**
			 * Here we are going to adapt the estimated unknown values
			 *
			 * @throws OrchestraException
			 */
			virtual void adaptEstimations()/* throw(OrchestraException)*/;

			/**
			 * This method first calculates the central values of the residuals for all
			 * functions Then checks if functions are convergent (residuals sufficiently
			 * close to zero)
		
			 * @throws OrchestraException
			 */
			virtual double howConvergent() /*throw(OrchestraException)*/;

			/**
			 * combined version of two routines from numerical recipes
			 */
			virtual void ludcmp_plus_lubksb(std::vector<std::vector<double>> &jac2, int const dim);
		};

	}

