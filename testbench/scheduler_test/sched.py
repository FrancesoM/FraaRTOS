#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Mar  5 00:21:42 2020

@author: francescomaio
"""
from __future__ import division
import random as rn

STATE_RUN = "RUN"
STATE_WAIT = "WAIT"

class Sim(object):
    def __init__(self,tasks):
        
        self.time = 0
        self.tasks = tasks
        self.tasks.append(Task("IDLE",10,100,100))
        self.idx_needs_schedul = []
        self.tasks[-1].request(0)
        #this to say start always from IDLE
        self.idle_id = len(self.tasks)-1
        self.curridx = self.idle_id
        
        
    def sim_request(self):
        #25% event
        if rn.randint(0,3) == 0:
            #add one random task simulating the request
            #but add only if task have been STATE - otherwise it cannot request again 
            
            #in order to do this creates an array of the possible threads that can request now
            #they cannot request if
            # - they are already running
            # - the deadline has not passed since they are arrived - how to impl?
            tasks_that_can_request = []
            for i in range(len(self.tasks)):
                if self.tasks[i].STATE == STATE_WAIT:
                    tasks_that_can_request.append(i)
            
            #now pick one randomly - if there is one
            if len(tasks_that_can_request) > 0:
                #get index
                random_idx = rn.randint(0,len(tasks_that_can_request)-1)
                #use index 
                self.tasks[tasks_that_can_request[random_idx]].request(self.time)
                print( "Request from thread {}".format(tasks[tasks_that_can_request[random_idx]].ID ) ) 
            else:
                print("No tasks can request -- all waiting to finish")
        else:
            print("No task requested at this time")    
        
    def check_if_missed(self):
        for t in self.tasks:
            if t.STATE == STATE_RUN:
                if self.time - t.arrive_time > t.dead:
                    raise Exception("Task {} has missed deadline".format(t))
          
    #the runlist is only a list of the idx-es of the tasks allowed to be scheduled
    def create_runlist(self):
        for i,t in enumerate(self.tasks):
            if t.STATE == STATE_RUN: 
                self.idx_needs_schedul.append(i)
                
                
    def clear_runlist(self):
        self.idx_needs_schedul = []
        
    def advance(self):
        print ("====== t: {} ======== ".format(self.time))
        self.check_if_missed()
        self.sim_request()
        self.create_runlist()
        print("==== run list: {} ====".format(self.idx_needs_schedul))
        self.schedule()
        self.clear_runlist()
        self.tasks[self.curridx].run()
        #print(tasks[self.curridx])
        self.time+=1
        
        
    def simulate(self,T):
        for t in range(T):
            self.advance()


    def schedule(self):
            
        #for tihs algorithm we travel the entire array anyway so no need to 
        #actually start from curridx
        
        """
        assume prio0 finishes and there are no others to be scheduled
        in theory the scheduler must let IDLE run but it cannot do it because
        """
        
        #if the current is not completed 
        if self.tasks[self.curridx].STATE == STATE_RUN:
            min_prio = self.tasks[self.curridx].prio
            min_dead = self.tasks[self.curridx].dead
            schedule_this = self.curridx
        #else take them from IDLE
        #the effect is that if idle is the only one in the run queue it will keep running
        #else it will preempted by any possible task
        else:
            min_prio = self.tasks[-1].prio
            min_dead = self.tasks[-1].dead
            schedule_this = self.idle_id
        


        for tnext in self.idx_needs_schedul:        
            
            
            tprio = self.tasks[tnext].prio
            tdead = self.tasks[tnext].dead
            
            
            if tprio == min_prio:
                #if the next deadline is shorter schedule this 
                if tdead < min_dead:
                    schedule_this = tnext
            
            #there is a task with higher priority    
            if tprio < min_prio:
                #update the min prio
                min_prio = tprio
                min_dead = tdead
                schedule_this = tnext
                


        print("Schedule from {} to {}".format( self.tasks[self.curridx].ID, self.tasks[schedule_this].ID )      )      
        self.curridx = schedule_this

        
        

class Task(object):
    def __init__(self,ID,prio,deadline,runtime):
        assert(deadline >= runtime)
        self.ID = ID
        self.prio = prio
        self.dead = deadline
        self.runtime = runtime
        self.time_has_run = 0
        #all start as being STATE_WAIT and they will arrive
        self.STATE = STATE_WAIT
        
    def request(self,time):
        self.arrive_time = time 
        self.time_has_run = 0
        self.STATE = STATE_RUN
        
    def run(self):
        self.time_has_run += 1
        print(self)
        if( self.time_has_run >= self.runtime ):
            print ( self.ID, "had been served <-----------------" )
            self.STATE = STATE_WAIT
        
    def __str__(self):
        perc = self.time_has_run/self.runtime if self.arrive_time != 0 else 0
        fill = int(perc*15)
        nofill = 15-fill
        return  """ 
===== ID: {}  ======
= prio {}, dead {} =
= target   = {}    =
= time run = {}    =
= arr time = {}    =
= [{}] =
                """.format( self.ID,
                            self.prio,
                            self.dead,
                            self.runtime,
                            self.time_has_run,
                            self.arrive_time,
                            ("*"*fill)+(" "*nofill))
    
if __name__ == "__main__":

    tasks = [    Task("0 HIGH",1,15,4),
                 Task("1 MEDIUM",2,20,10),
                 Task("2 CRITICAL",0,12,4),
                 Task("3 MEDIUM",2,11,3),
                 Task("4 CRITICAL",0,13,2)]
    
    mySimulation = Sim(tasks)
    mySimulation.simulate(20)



    

#print("schedule this: \t'{}'\nfrom this \t'{}'".format(tasks[schedule_this],tasks[curr]))