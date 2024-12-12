#include "UnEqGroup.h"
#include "UnEq.h"
#include "VarGroup.h"
#include "IO.h"
#include "OrchestraException.h"
#include "FileBasket.h"
#include "stringhelper.h"

namespace orchestracpp
{



	UnEqGroup::UnEqGroup(VarGroup *variables)
	{
		this->variables = variables;
	}

	void UnEqGroup::initialise()
	{
		int olddim = nrActiveUneqs;
		nrActiveUneqs = 0;

		// is it necessary to re-create this array for each iteration?
		// or can we create one that is large enough once?
		if (activeUneqs.empty())
		{
			activeUneqs.resize(uneqs.size() + 1);
		}

		// Create the list of active uneqs
		for (auto tmp : uneqs)
		{
			if (tmp->active)
			{
				activeUneqs[nrActiveUneqs + 1] = tmp;
				nrActiveUneqs++;
			}
		}

		// dimension the jacobian matrix according to the number of active uneqs
		// only create a new one if nr active uneqs has changed
		if (nrActiveUneqs != olddim || jacobian.empty())
		{
			jacobian = *new std::vector<std::vector<double>>(nrActiveUneqs + 1);

			for (int vector1 = 0; vector1 < (nrActiveUneqs + 1); vector1++)
			{
				jacobian[vector1] = std::vector<double>(nrActiveUneqs + 1);
			}
		}

		if (minTol == nullptr) {
			minTol = variables->get("minTol");
			if (minTol != nullptr) {
				minTol->setConstant(false);
			}
		}

		minTolOrgValue = minTol->getValue();

		if (tolerance == nullptr) {
			tolerance = variables->get("tolerance");
			if (tolerance != nullptr) {
				tolerance->setConstant(false);
			}
		}
	}

	UnEq *UnEqGroup::doesExist(UnEq *u) //throw(ReadException)
	{
		for (auto x : uneqs)
		{
			if (StringHelper::equalsIgnoreCase(x->unknown->name, u->unknown->name) ||
				StringHelper::equalsIgnoreCase(x->equation->name, u->unknown->name) ||
				StringHelper::equalsIgnoreCase(x->unknown->name, u->equation->name) ||
				StringHelper::equalsIgnoreCase(x->equation->name, u->equation->name))
			{
				throw ReadException("Uneq: " + u->unknown->name + " already exists");
			}
		}
		return u;
	}

	void UnEqGroup::read_one2(const std::string &infile) //throw(ReadException, IOException)
	{
		uneqs.push_back(doesExist(UnEq::createUnEq2(infile, variables)));
	}

	void UnEqGroup::read_one3(const std::string &infile)// throw(ReadException, IOException)
	{
		uneqs.push_back(doesExist(UnEq::createUnEq3(infile, variables)));
	}

 /**
  * This is the top level iteration method that is called from the calculator
  * and manages the iteration process the calculate method of the calculator
  * is used as a call-back method to perform a single calculation in the
  * iteration process.
  *
  * iterate
  *     iteratelevelminerals
  *         iteratelevel0
  */

	bool UnEqGroup::iterate(StopFlag *flag)
	{
		originalMaxIter = maxIter;
		totalNrIter = 1;

		try
		{
			iterateLevelMinerals(flag);
		}
		catch (const IOException &ioe)
		{
			IO::showMessage(ioe.what());
		}

		return (nrIter < maxIter);
	}

	double UnEqGroup::getTotalNrIter()
	{
		return totalNrIter;
	}

	double UnEqGroup::getNrIter()
	{
		return nrIter;
	}

	bool UnEqGroup::getIIApresent()
	{
		for (auto u : uneqs) {
			if ((u->initiallyInactive) && (!u->active)) {
				return true;
			}
		}
		return false;
	}

	void UnEqGroup::switchOnIIA()
	{
		for (auto u : uneqs) {
			if (u->initiallyInactive) {
				if (!u->active) {
					IO::println("Switching on: " + u->unknown->name + ": " + StringHelper::toString(u->unknown->getIniValue()));
					u->active = true;
				}
			}
		}
	}

	void UnEqGroup::switchOffIIA()
	{
		for (auto u : uneqs) {
			if (u->initiallyInactive) {
				if (u->active) {
					IO::println("Switching off: " + u->unknown->name + ": " + StringHelper::toString(u->unknown->getIniValue()));
					u->active = false;
				}
			}
		}
	}

	void UnEqGroup::iterateLevelMinerals(StopFlag *flag)// throw(IOException)
	{
		if (monitor)
		{
			initialiseIterationReport();
		}

//		if (firstIteration2)
//		{
//			initialiseIterationReport2();
//		}

		int nrMineralIteration = 0;
		bool mintolflipped = false;

		// activate - inactivate uneq3's based on given values of unknown
		// negative values will switch uneq off
		// we keep this set constant during a mineral iteration

		int nrOfMinerals = 0;

		for (auto uneq : uneqs)
		{
			if (uneq->isType3)
			{
				nrOfMinerals++;
				uneq->active = uneq->unknown->getValue() > 0;
			}
		}
		
		maxMineralIterations = std::max(50, nrOfMinerals);

		while (nrMineralIteration < maxMineralIterations)
		{
			nrMineralIteration++;
			bool mineralCompositionChanged = false;

			nrIter = iterateLevel0(iterationReport, flag); // <------------------------------------------

			// if nrIter = maxNrIter, then we did not find convergence, what are doing with this info?
			
            // find the most supersaturated INACTIVE mineral

			UnEq *mostSuperSatInactiveUnEq = nullptr;
			double mostsat = 0;

			for (auto uneq : uneqs)
			{
				if (uneq->isType3)
				{
					double satindex = uneq->siVariable->getValue();
					if (!uneq->active)
					{
						if (satindex > mostsat)
						{
							mostsat = satindex;
							mostSuperSatInactiveUnEq = uneq;
						}
					}
				}
			}

			if (mostsat > 0)  // we found a supersaturated inactive mineral, set active
			{
				mineralCompositionChanged = true;

				if (!mintolflipped)
				{
					mintolflipped = true;
					minTol->setValue(1e-3); // we only set the value of mintol to 1e-3 once during iteration
				}
				mostSuperSatInactiveUnEq->active = true;
				mostSuperSatInactiveUnEq->unknown->setValue(1e-3);
			}

			if (flag != nullptr)
			{
				if (flag->cancelled)
				{
					break;
				}
			}

			if (!mineralCompositionChanged)
			{
				if (minTol->getValue() > 0.0) {
					minTol->setValue(0);
				}
				else {
					// mineralcomposition has not changed, and mintol = zero so we are ready
					break; /****  This is the place for a successful exit ****/
				}
			}


			if (nrMineralIteration >= maxMineralIterations)
			{
				IO::println("****** max nr min iterations, no solution found   ");
				break;
			}


		}


		if ((monitor) && (iterationReport != nullptr))
		{
			iterationReport->close();
			monitor = false;
		}

//		if ((firstIteration2) && (iterationReport != nullptr))
//		{
//			iterationReport->close();
//			firstIteration2 = false;
//		}


	}

	int UnEqGroup::iterateLevel0(Writer *iterationReportWriter, StopFlag *flag)
	{
		int nrIter0 = 1;
		initialise();
		// Here we create the actual matrix of active uneqs that        
		// is used during the iteration 

		try
		{
			nrIter0 = 1;

			if (nrActiveUneqs == 0)
			{
				return nrIter0;
			}
			howConvergent_field = 0;
			try
			{
				while ((howConvergent_field = howConvergent()) > 1)
				{
					if ((monitor) && (iterationReportWriter != nullptr))
					{
						writeIterationReportLine(nrIter);
					}

//					if ((firstIteration2) && (iterationReportWriter != nullptr))
//					{
//						writeIterationReportLine2(totalNrIter);
//					}

					calculateJacobian();
					adaptEstimations();
					nrIter0++;
					totalNrIter++;

					if (flag != nullptr)
					{
						if (flag->cancelled)
						{ // direct access of volatile variable is faster
							nrIter0 = maxIter;
							break;
						}
					} 


					if ((nrIter0 >= maxIter))
					{
						nrIter0 = (int)maxIter;
						break;
					}
				}
			}
			catch (const OrchestraException &e)
			{
				// something went wrong during the iterations
				//IO::println(e.what());
				nrIter0 = (int)maxIter; // this will cause iteration to stop and indicate failure
			}

		}
		catch (const IOException &ioe)
		{
			//do we want an interactive message here?
			IO::showMessage(ioe.what());
		}
		return nrIter0;
	}

	void UnEqGroup::initialiseIterationReport()// throw(IOException)
	{
		iterationReport = FileBasket::getFileWriter(nullptr, "iteration.dat");


		Var* tmp = variables->get("Node_ID");
		double value;
		std::string valuestring;
		if (tmp != nullptr) {
			value = tmp->getIniValue();
			valuestring = StringHelper::toString(value);
		}


		iterationReport->write("NodeID: " + valuestring + "\n");

		iterationReport->write(IO::format("nr", 5));
		iterationReport->write(IO::format("logfactor", 20));
		iterationReport->write(IO::format("convergence", 20));
		for (auto uneq : uneqs)
		{
			iterationReport->write(IO::format(uneq->unknown->name, 25));
			iterationReport->write(IO::format("   ", 3));
			iterationReport->write(IO::format(uneq->equation->name, 20));
		}
		iterationReport->write("\n");

		iterationReport->write(IO::format("   ", 45)); // empty
		for (auto uneq : uneqs)
		{
			iterationReport->write(IO::format("   ", 25));
			iterationReport->write(IO::format("   ", 3));
			iterationReport->write(IO::format(uneq->equation->getIniValue(), 20, 8));
		}

		iterationReport->write("\n");
	}
	/*
	void UnEqGroup::initialiseIterationReport2()// throw(IOException)
	{
		iterationReport = FileBasket::getFileWriter(nullptr, "iterationcpp.dat");
		iterationReport->write(variables->getVariableNamesLine());
		iterationReport->write('\n');
	}
	*/
	void UnEqGroup::writeIterationReportLine(double nrIter) //throw(IOException, OrchestraException)
	{
		if (totalNrIter > 1000)
		{
			return;
		}
		if (nrIter == 1)
		{
			iterationReport->write("\n");
		}
		iterationReport->write(IO::format(StringHelper::toString(nrIter), 5));
		iterationReport->write(IO::format(StringHelper::toString(std::log10(commonfactor)), 20));
		iterationReport->write(IO::format(StringHelper::toString(std::log10(howConvergent_field)), 20));

		for (auto uneq : uneqs)
		{
			try
			{
				iterationReport->write(IO::format(uneq->unknown->getIniValue(), 25, 8));
			}
			catch (const IOException &e1)
			{
				iterationReport->write(IO::format("NaN", 25));
			}

			// this values is currently not used
			//std::log10(uneq->howConvergent());

			if (uneq->isConvergent())
			{
				iterationReport->write(IO::format("  ", 3));
				// iterationReport.write(IO.format(Math.log10(uneq.howConvergent()), 5, 1));
				// iterationReport.write(IO.format(Math.log10(uneq.tolerance()), 5, 1));
			}
			else
			{ // not convergent
				if (uneq->isType3)
				{
					if (uneq->active)
					{
						iterationReport->write(IO::format("X ", 3));
						//iterationReport.write(IO.format(Math.log10(uneq.howConvergent()), 5, 1));
					}
					else
					{
						//UnEq3 *tmp = static_cast<UnEq3*>(uneq);
						if (uneq->siVariable->getValue() > 0)
						{
							iterationReport->write(IO::format("S ", 3));
							//iterationReport.write(IO.format(Math.log10(uneq.howConvergent()), 5, 1));
						}
						else
						{
							iterationReport->write(IO::format("  ", 3));
							//iterationReport.write(IO.format(Math.log10(uneq.howConvergent()), 5, 1));
						}
					}
				}
				else
				{
					iterationReport->write(IO::format("X ", 3));
					//iterationReport.write(IO.format(Math.log10(uneq.howConvergent()), 5, 1));
				}
			}

			uneq->calculateCentralResidual();

			try
			{
				iterationReport->write(IO::format(uneq->equation->getValue(), 20, 8));
			}
			catch (const IOException &e1)
			{
				iterationReport->write(IO::format("NaN", 20));
			}
		}
		iterationReport->write("\n");
	}
	/*
	void UnEqGroup::writeIterationReportLine2(double nrIter) //throw(IOException, OrchestraException)
	{
		if (nrIter > 120) {
			return;
		}
		iterationReport->write(variables->getVariableValuesLine());
		iterationReport->write('\n');
    }
    */

	void UnEqGroup::calculateJacobian()// throw(OrchestraException)
		{

			for (int i = 1; i <= nrActiveUneqs; i++)
			{

				// store the original unknown value
				// and offset the unknown value input
				double originalUnknownValue = activeUneqs[i]->offsetUnknown();

				// calculate the residuals for the offset of this unknown
			    for (int m = 1; m <= nrActiveUneqs; m++) {
			        activeUneqs[m]->calculateJResidual();
			    }

				// reset the unknown to original value
				activeUneqs[i]->resetUnknown(originalUnknownValue);

				// calculate the jacobian values from the residuals
				for (int fnr = 1; fnr <= nrActiveUneqs; fnr++)
				{
					jacobian[fnr][i] = (activeUneqs[fnr]->jacobianResidual - activeUneqs[fnr]->centralResidual) / activeUneqs[i]->un_delta;
				}
			}

		}

		void UnEqGroup::adaptEstimations() //throw(OrchestraException)
		{

			ludcmp_plus_lubksb(jacobian, nrActiveUneqs + 1);

			/**
			 * Determine the maximum common factor for changing the unknowns in the
			 * iteration process If the required maximum factor (as determined in
			 * check unknown step) for one of the unknowns is smaller than extremely
			 * small
			 */
			commonfactor = 1;
			double minimumfactor = 1e-5;

			for (int m = 1; m <= nrActiveUneqs; m++)
			{
			// If  factor < 1, factor is reduced by checkunknownstep
				// we use the smallest factor to update all unknowns, so keep original 
				// direction.
				double factor = activeUneqs[m]->checkUnknownStep();
				// unless one of the factors is extremely small
				// this means that the result is very sensitive for change in unknown?
				if ((factor > minimumfactor) && (factor < commonfactor))
				{
					commonfactor = factor;
				}
			}

			for (int m = 1; m <= nrActiveUneqs; m++)
			{
				if (activeUneqs[m]->factor < commonfactor)
				{
					// is it better to update unknowns that are very sensitive with
					// their own small factor? 
					activeUneqs[m]->updateUnknown(activeUneqs[m]->factor);
				}
				else
				{
					activeUneqs[m]->updateUnknown(commonfactor);
				}
			}
		}

		double UnEqGroup::howConvergent()// throw(OrchestraException)
		{

			double convergence = 0;

			for (int m = 1; m <= nrActiveUneqs; m++)
			{
				activeUneqs[m]->calculateCentralResidual();
				convergence = std::max(convergence, activeUneqs[m]->howConvergent());
			}

			return (convergence);
		}

		void UnEqGroup::ludcmp_plus_lubksb(std::vector<std::vector<double>> &jac2, int const dim)
		{
			std::vector<double> vv(dim);
			std::vector<int> indx(dim);

			for (int i = 1; i < dim; i++)
			{
				double big = 0.0;
				for (int j = 1; j < dim; j++)
				{
					double temp;
					if ((temp = std::abs(jac2[i][j])) > big)
					{
						big = temp;
					}
				}
				if (big == 0.0)
				{
					return;
				}
				vv[i] = 1.0 / big;
			}

			for (int j = 1; j < dim; j++)
			{
				int imax = 1;

				for (int i = 1; i < j; i++)
				{
					for (int k = 1; k < i; k++)
					{
						jac2[i][j] -= jac2[i][k] * jac2[k][j];
					}
				}

				double big = 0.0;
				for (int i = j; i < dim; i++)
				{
					for (int k = 1; k < j; k++)
					{
						jac2[i][j] -= jac2[i][k] * jac2[k][j];
					}

					double dum;
					if ((dum = vv[i] * std::abs(jac2[i][j])) >= big)
					{
						big = dum;
						imax = i;
					}
				}
				if (j != imax)
				{
					std::vector<double> dum;
//					double dum;
					dum = jac2[imax];
					jac2[imax] = jac2[j];
					jac2[j] = dum;

					vv[imax] = vv[j];
				}
				indx[j] = imax;

				//------------------            
				if ((jac2[j][j]) == 0.0)
				{
				   //IO.showMessage("matrix is singular!");
				   //IO.println("Jacobian matrix is singular!");
				   jac2[j][j] = 1e-30;
				}
				//------------------------------------------

				if (j != dim - 1)
				{
					double dum = 1.0 / (jac2[j][j]);
					for (int i = j + 1; i < dim; i++)
					{
						jac2[i][j] *= dum;
					}
				}
			}

			int ii = 0;
			for (int i = 1; i < dim; i++)
			{
				int ip = indx[i];
				double sum = activeUneqs[ip]->centralResidual;
				activeUneqs[ip]->centralResidual = activeUneqs[i]->centralResidual;
				if (ii != 0)
				{
					for (int j = ii; j <= i - 1; j++)
					{
						sum -= jac2[i][j] * activeUneqs[j]->centralResidual;
					}
				}
				else if (sum != 0)
				{
					ii = i;
				}
				activeUneqs[i]->centralResidual = sum;
			}

			for (int i = dim - 1; i >= 1; i--)
			{
				double sum = activeUneqs[i]->centralResidual;
				for (int j = i + 1; j < dim; j++)
				{
					sum -= jac2[i][j] * activeUneqs[j]->centralResidual;
				}
				activeUneqs[i]->centralResidual = sum / jac2[i][i];
			}
		}
	}
