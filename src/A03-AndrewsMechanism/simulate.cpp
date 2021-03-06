// This is part of
// Multi-Body Systems Benchmark in OpenSim (MBS-BOS)
// Copyright (C) 2014 Luca Tagliapietra Michele Vivian Monica Reggiani
//
// MBS-BOS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MBS-BOS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MBS-BOS.  If not, see <http://www.gnu.org/licenses/>.
//
// The authors may be contacted via:
// email: tagliapietra@gest.unipd.it

#include <iostream>
    using std::cout;
    using std::endl;
#include "OpenSim/OpenSim.h"
#include "simulationManager.h"
#include "configurationInterpreter.h"
#include "DcMotorPIDController.h"


int main(int argc, char **argv) {
  cout << "--------------------------------------------------------------------------------" << endl;
  cout << " Multi-Body System Benchmark in OpenSim" << endl;
  cout << " Benchmark reference url: http://lim.ii.udc.es/mbsbenchmark/" << endl;
  cout << " Problem A03: Andrew's Mechanism Simulator" << endl;
  cout << " Copyright (C) 2013, 2014 Luca Tagliapietra, Michele Vivian, Monica Reggiani" << endl;
  cout << "--------------------------------------------------------------------------------" << endl;

  if (argc != 2){
    cout << " ******************************************************************************" << endl;
    cout << " Multi-Body Systems Benchmark in Opensim: Simulator for Model A03" << endl;
    cout << " Usage: ./AndrewsMechanismSimulate dataDirectory" << endl;
    cout << "       dataDirectory must contain a vtpFiles folder" << endl;
    cout << " ******************************************************************************" << endl;
    exit(EXIT_FAILURE);
  }
  
  const std::string dataDir = argv[1];
  cout << "Input data directory: " << dataDir << endl;

  const std::string outputDir = dataDir+"/SimulationResults";
  
  // Load the Opensim Model
  OpenSim::Model andrewsMechanism((dataDir+"/AndrewsMechanism.osim").c_str());

  SimTK::Vec3 pointA(0,0,0);
  SimTK::Vec3 pointB(0,0,1);
  
  SimTK::Vec3 axis = pointA - pointB;
  OpenSim::TorqueActuator *motor = new OpenSim::TorqueActuator(andrewsMechanism.updBodySet().get(std::string("ground")), andrewsMechanism.updBodySet().get(std::string("OF")),axis, true);
  motor->setName("motor");
  motor->set_optimal_force(0.033);
  andrewsMechanism.addForce(motor);
  
  int indexMotor = andrewsMechanism.updActuators().getIndex("motor");
  DcMotorPIDController *constController = new DcMotorPIDController(andrewsMechanism, 0.0, 0.0, 0.0,indexMotor); //0.5
  constController->setName("Constant Controller");
  constController->setActuators(andrewsMechanism.updActuators());
  andrewsMechanism.addController(constController);

  // Add Force reporter and kinematics reporter to the model  
  OpenSim::ForceReporter *forceReporter = new OpenSim::ForceReporter(&andrewsMechanism);
  forceReporter->setName(std::string("forceReporter"));
  andrewsMechanism.addAnalysis(forceReporter);
  
  OpenSim::PointKinematics *pointKinematicsReporter = new OpenSim::PointKinematics(&andrewsMechanism);
  pointKinematicsReporter -> setBodyPoint(std::string("OF"), SimTK::Vec3(0.0035,0,0));
  pointKinematicsReporter->setName(std::string("pointKinematicsReporter"));
  pointKinematicsReporter ->setDescription("3d Kinematics of the point F (state_0 = X Displacement, state_1 = Y Displacement, state_2 = Z Displacement)");
  andrewsMechanism.addAnalysis(pointKinematicsReporter);
  
  // Read the configuration Parameter File
  std::map<std::string, double> parametersMap;
  try{
    const std::string cfgFilename = (dataDir+"/simulationParameters.txt");
    configurationInterpreter cfg(cfgFilename.c_str());
    cfg.getMap(parametersMap);
  }
  catch (std::exception& e){
    std::cerr << e.what() << std::endl;
    return -1;
  }
  
  SimTK::State fakedInitialState;
  simulationManager manager(fakedInitialState, andrewsMechanism, parametersMap, "RungeKuttaMerson", outputDir);
  manager.simulate();
  
  cout << "Simulation results stored in: " << outputDir << endl;
}
