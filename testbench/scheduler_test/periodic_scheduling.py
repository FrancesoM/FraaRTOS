#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from IPython import get_ipython
get_ipython().run_line_magic('matplotlib', 'qt')


import numpy as np
import matplotlib.pyplot as plt
import copy

"""
Created on Wed Apr 29 22:59:47 2020

@author: francescomaio
"""

"""
Some nomenclature
- Arrival time ai is the time at which a task becomes ready for execution; it is also
   referred as request time or release time and indicated by ri;
- Computation time Ci is the time necessary to the processor for executing the
   task without interruption;
- Absolute Deadline di is the time before which a task should be completed to
   avoid damage to the system;
- Relative Deadline Di is the difference between the absolute deadline and the
   request time: Di = di − ri;
- Start time si is the time at which a task starts its execution;
- Finishing time fi is the time at which a task finishes its execution;
- Response time Ri is the difference between the finishing time and the request
   time: Ri = fi − ri;

Nomenclature for periodic tasks
TAU denotes a set of periodic tasks;
tau_i denotes a generic periodic task;
tau_i,j denotes the jth instance of task tau_i;
ri,j denotes the release time of the jth instance of task tau_i;
PHI_i denotes the phase of task tau_i; that is, the release time of its first instance
(PHI_i = ri,1);
Di denotes the relative deadline of task tau_i;
di,j denotes the absolute deadline of the jth instance of task tau_i (di,j = PHI_i +
(j − 1)Ti + Di).
si,j denotes the start time of the jth instance of task tau_i; that is, the time at
which it starts executing.
fi,j denotes the finishing time of the jth instance of task tau_i; that is, the time
at which it completes the execution.

Assumptions - Then each algorithm could embrace all the assumptions or release 
some of them

- A1. The instances of a periodic task tau_i are regularly activated at a constant
      rate. The interval Ti between two consecutive activations is the period
      of the task.
- A2. All instances of a periodic task tau_i have the same worst-case execution
      time Ci.
- A3. All instances of a periodic task tau_i have the same relative deadline Di,
      which is equal to the period Ti.
- A4. All tasks in Γ are independent; that is, there are no precedence relations
      and no resource constraints.

In addition, the following assumptions are implicitly made:
    
- A5. No task can suspend itself, for example on I/O operations.
- A6. All tasks are released as soon as they arrive.
- A7. All overheads in the kernel are assumed to be zero.

Now, we need a metric to define the feasibility of a scheduling coming from a given algorithm

The book proposes the Processor utilization factor:
    
            U = sum( C_i/T_i ) for i in set of tasks
    
Let U_ub(TAU,A) the "upper bound" utilization factor created by a given algorithm
on a given set of tasks. When U = U_ub it means that the slightest increase in 
one task computation time will make the schedule infeasible.

No feasible schedule has U > 1, but to better understand this metric figure we 
need to understand that U_ub can be smaller than 1. It can happen that the 
best scheduling does not have U = 1 because of how C and T happen to be. This
means that the produced schedule has "gaps" in which the processor is IDLE but
we can't do better than that. This may be due to the algorithm not being perfect
or because the particular configuration of C and T in TAU. 

Since we need this metric to analyze how good an algorithm is, we notice that 
U is still depending on TAU and A. In order to get TAU out of the picture we 
must define U_lub which is defined as:
    
            U_lub(A) = min (over all tau in TAU) of U_ub(TAU,A)

Because we want our algorithm to work on an arbitrarily set of tasks. This metric
is used in the analysis and also when an algorithm is applied

Analysis: generate a random set of tasks and minimize U_ub 

Application: schedule the set of tasks with A and if the produced U is lower than 
             U_lub, then the schedule is feasible. If the produced U is higher
             then U_lub but smaller than 1, then we can't say anything about 
             the feasibility of the schedule. 
             
This is because in calculating U_lub the worst case is always taken into account. 
Referring to the proofs in the book, then I report here some useful results:
    
Rate Monotonic scheduling => U_lub = n*(2^(1/n) - 1), and with n high it approached 0.69
Earliest Deadline First   => U_lub = 1 

Relaxing assumption 3: "Period equals deadline" 

Explanation of the algorithm as it's not as straightforward as the two previous ones
    - A phase PHI_i;
    - A worst-case computation time Ci (constant for each instance);
    - A relative deadline Di (constant for each instance);
    - A period Ti.
    
The parameters have the following relationship:
    
    Ci ≤ Di ≤ Ti

This is easy to explain: the computation time is always smaler than its deadline (obviously otherwise)
the task is flawed to begin with, and the period of course is bigger of the others two.     

    ri,k = PHI_i + (k − 1)Ti
    
(k-1)Ti: is the start time of the (k-1)th instance but then it might happen that not all periods
start at the same time so we take into account this with PHI_i

    di,k = ri,k + Di.

The scheduling algorithm assigns priority inversely proportional to a task relative deadline, so we could
use a modified RM U_lub and say that a schedule is feasible if:
    
                        U_lub = sum(Ci/Di) (note that Ti is substituted by Di)

But this would give a pessimistic estimation. We can have a better estimation noticing the following:
    
- 
    


"""
COLORS = [
'b' 	,
'g' 	,
'r' 	,
'c' 	,
'm' 	,
'y' 	,
'k' 	,
'w' 	]


#different ways of scheduling a set of tasks
# ======================== TASK CLASS ========================= #

class Task(object):
    
    SCALE = {"s" : 1, "ms": 0.001, "us": 0.000001}
    
    def __init__(self,ID,runtime,period,deadline,scale="ms"):
        assert(deadline >= runtime)
        self.ID = ID
        self.D = int(deadline)
        self.C = int(runtime)
        self.T = int(period)
        self.scale = Task.SCALE[scale]
        self.time_has_run = 0
        self.running = False
        
    def run(self,time):
        #print("Running " + self.ID)
        self.running = True
        #increase by 1 unit of time  
        self.time_has_run += time
        has_completed = False
        if self.time_has_run >= self.C :
            #reset time has run
            self.time_has_run = 0
            has_completed = True
            self.running = False
            #print(self.ID + " has finished" )
            
        return has_completed

    def __str__(self):
        return self.ID
    
    __repr__ = __str__


#class to draw the scheduling
class Display(object):
    def __init__(self):
        self.release = []
        self.schedule = []
        self.dead = []
        
    def draw_sched_line(self,h,x1,x2,c):
        #plot an horizontal segment from t0 to t1 
        #the "y" is the task number
        self.schedule.append((x1,x2))
        self.schedule.append((h,h))
        self.schedule.append(c)
        
    def draw_release_line(self,x,h1,h2,c):
        #plot an horizontal segment from t0 to t1 
        #the "y" is the task number
        self.release.append((x,x))
        self.release.append((h1,h2))
        self.release.append(c)
        
    def draw_dead_line(self,x,h1,h2,c):
        #plot an horizontal segment from t0 to t1 
        #the "y" is the task number
        self.dead.append((x,x))
        self.dead.append((h1,h2))
        self.dead.append(c)
        
        
    def plot(self,ymax,tmax):
        
        #print(self.schedule)
        
        plt.ylim(-0.5,ymax)
        plt.xlim(0,tmax)
        
        plt.plot(*self.schedule,linewidth=5)
        plt.plot(*self.release,linewidth=2)
        plt.plot(*self.dead,linewidth=2)
        
        
        #plt.plot(*release,linewidth=2)
        plt.show()
                    
        

#the static scheduler provides a scheduling for a period which is then
#repeated ever and ever again 
class DM_Scheduler():
    
    def __init__(self,_taskslist):
        
        self.Display = Display()
        self.TAU = copy.deepcopy(_taskslist)
        
        print(self.find_task_number_from_ID("D"))
        
        #rescale all the tasks to have the same timescale
        self.smallest_scale  = min(self.TAU, key=lambda tau : tau.scale).scale
        
        for i,tau in enumerate(self.TAU):
            factor = tau.scale/self.smallest_scale
            self.TAU[i].D = int(tau.D * factor)
            self.TAU[i].C = int(tau.C * factor)
            self.TAU[i].T = int(tau.T * factor)
            self.TAU[i].scale = self.smallest_scale
        
        #sort tasks based on deadline and priority follows this scheme
        self.TAU.sort(key=lambda tau : tau.D )
        
        #now find the smallest unit of time and normalize periods
        self.smallest_period  = min(self.TAU, key=lambda tau : tau.T).T
        self.smallest_runtime = min(self.TAU, key=lambda tau : tau.C).C
              
        
        #assuming t=0 is the critical instant then find the repetition time
        #aka when the scheduling repeats itself, which is the least common multiplier
        self.repetition_period = np.lcm.reduce([tau.T for tau in self.TAU])
        
        #max deadline -> last element
        #add convenient IDLE task
        self.TAU.append(Task("IDLE", #ID
                             self.repetition_period, #C
                             self.repetition_period, #T
                             self.repetition_period  #D so always last
                             )     
                         )
        
        ready_queue = copy.deepcopy(self.TAU)
        t = 0
        t_step = 1 #<--- if you change this make sure to change logic for checking refill ready queue
        
        self.schedule = []
        
        #now propose the DM schedule - simulate all the instants
        while  t < self.repetition_period:
            
            #print("ready" , ready_queue)
            self.schedule.append((ready_queue[0].ID,
                                  self.find_task_number_from_ID(ready_queue[0].ID),
                                  t))
            #print("schedule" , self.schedule)
        
            #let the highest priority task run and remove it if it finishes
            
            #run returns True if it has finished
            if ready_queue[0].run(t_step) :
                ready_queue.pop(0)
                
            #increase simulation time
            t+=t_step
                
            #fill the ready queue - to do this just see if any task period has finished at the 
            #current t
            for tau in self.TAU:
                #if the time is aligned to the period repetition
                #print("t: {} mod Period {} -> {}".format(t,tau.T,t%tau.T))
                if t%tau.T == 0:
                    if tau.running == False:
                        ready_queue.append(tau)
            
            #resort the queue to fix priorities
            ready_queue.sort(key=lambda tau : tau.D )
                
        
        

    def find_task_number_from_ID(self,ID):
        return [i for i,tau in enumerate(self.TAU) if tau.ID == ID][0]
        
    
    def drawSchedule(self):
        
        #print(self.schedule)
        for s in self.schedule:
            
            h = s[1]
            t = s[2]
            c = COLORS[h%len(COLORS)]
            self.Display.draw_sched_line(h,t,t+1,c)
            
        #now draw periods
        
        for tau in self.TAU:
            
            h = self.find_task_number_from_ID(tau.ID)
            c = "-" + COLORS[h%len(COLORS)]
            activations = np.arange(0,self.repetition_period,tau.T)
            for a in activations:
                self.Display.draw_release_line(a,h,h+0.3,"k")
                self.Display.draw_dead_line(a+tau.D,h,h+0.3,c)
        
        
        self.Display.plot(len(self.TAU),len(self.schedule))
 


if __name__ == "__main__":
        
    """
       Ci Ti Di
    τ1 1 4 3
    τ2 1 5 4
    τ3 2 6 5
    τ4 1 11 10
    """
    
    A = Task("A",1,4,3)
    B = Task("B",1,5,4)
    C = Task("C",2,6,5)
    D = Task("D",1,11,10)
    
    TAU = [A,B,C,D]
    
    S = DM_Scheduler(TAU)
    
    S.drawSchedule()
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    
    
    