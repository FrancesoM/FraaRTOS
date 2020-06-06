#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun  6 23:00:56 2020

@author: francescomaio
"""

import scheduler_simulator as sim
import periodic_schedulers as ps

        
"""
   Ci Ti Di
τ1 1 4 3
τ2 1 5 4
τ3 2 6 5
τ4 1 11 10
"""

A = sim.PeriodicTask("A",1,4,3)
B = sim.PeriodicTask("B",1,5,4)
C = sim.PeriodicTask("C",2,6,5)
D = sim.PeriodicTask("D",1,11,10)

TAU = [A,B,C,D]

#Test DM 
"""
S = OS_Simulator(TAU,
       OS_Simulator.DM_Scheduler,
       OS_Simulator.DM_Guarantee,
       15)
"""

#Test EDF
S = sim.OS_Simulator(TAU,
      ps.EDF_Scheduler,
      ps.EDF_Guarantee,
      15)


S.simulate()

S.drawSchedule()